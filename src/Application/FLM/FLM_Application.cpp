/**
 * @file FLM_Application.cpp
 * @brief FLM Application SWC Implementation
 * @details Main control logic determining headlight activation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - Main FLM control logic
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "FLM_Application.h"
#include "Application/SwitchEvent/SwitchEvent.h"
#include "Application/LightRequest/LightRequest.h"
#include "Dem_Cfg.h"
#include <cstring>

/*============================================================================*
 * LOCAL MACROS
 *============================================================================*/

/** @brief Degraded mode timeout in cycles */
#define FLM_DEGRADED_TIMEOUT_CYCLES     ((FLM_FTTI_MS - FLM_SAFE_STATE_TRANSITION_MS) / FLM_MAIN_FUNCTION_PERIOD_MS)

/** @brief E2E timeout cycles */
#define FLM_E2E_TIMEOUT_CYCLES          (FLM_E2E_TIMEOUT_MS / FLM_MAIN_FUNCTION_PERIOD_MS)

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Component internal state */
static FLM_Application_StateType FLM_State;

/** @brief System time counter */
static uint32_t FLM_SystemTime = 0U;

/** @brief Safe state trigger from external */
static boolean FLM_ExternalSafeStateTrigger = FALSE;
static SafeStateReason FLM_SafeStateReason = SAFE_STATE_REASON_NONE;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void FLM_ReadInputs(void);
static void FLM_ProcessStateMachine(void);
static void FLM_StateInit(void);
static void FLM_StateNormal(void);
static void FLM_StateDegraded(void);
static void FLM_StateSafe(void);
static void FLM_DetermineHeadlightCommand(void);
static void FLM_ApplyAutoMode(void);
static void FLM_ReportWdgMCheckpoint(void);
static void FLM_ReportDemEvents(void);
static boolean FLM_AreAllInputsValid(void);
static boolean FLM_IsAnyInputInvalid(void);
static boolean FLM_IsCriticalFault(void);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize FLM Application component
 */
void FLM_Init(void) {
    /* Clear state structure */
    (void)memset(&FLM_State, 0, sizeof(FLM_State));

    /* Initialize state machine in INIT state */
    FLM_State.currentState = FLM_STATE_INIT;
    FLM_State.previousState = FLM_STATE_INIT;
    FLM_State.stateEntryTime = 0U;

    /* Initialize input data */
    FLM_State.lightSwitch.command = LIGHT_SWITCH_OFF;
    FLM_State.lightSwitch.isValid = FALSE;
    FLM_State.ambientLight.isValid = FALSE;
    FLM_State.switchSignalStatus = SIGNAL_STATUS_INVALID;
    FLM_State.ambientSignalStatus = SIGNAL_STATUS_INVALID;

    /* Initialize output */
    FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;

    /* Initialize hysteresis */
    FLM_State.lightsCurrentlyOn = FALSE;
    FLM_State.hysteresisActive = FALSE;

    /* Initialize error tracking */
    FLM_State.consecutiveErrors = 0U;
    FLM_State.e2eTimeoutActive = FALSE;

    /* Reset external triggers */
    FLM_ExternalSafeStateTrigger = FALSE;
    FLM_SafeStateReason = SAFE_STATE_REASON_NONE;

    /* Mark as initialized */
    FLM_State.isInitialized = TRUE;
}

/**
 * @brief Main function for FLM Application
 */
void FLM_MainFunction(void) {
    if (!FLM_State.isInitialized) {
        return;
    }

    /* Report entry checkpoint to WdgM [SysSafReq03] */
    FLM_ReportWdgMCheckpoint();

    /* Update timestamp */
    FLM_State.currentTime = FLM_SystemTime;
    FLM_SystemTime += FLM_MAIN_FUNCTION_PERIOD_MS;

    /* Read inputs from other SWCs */
    FLM_ReadInputs();

    /* Process state machine [FunSafReq01-03] */
    FLM_ProcessStateMachine();

    /* Determine headlight command based on state */
    FLM_DetermineHeadlightCommand();

    /* Report DEM events */
    FLM_ReportDemEvents();
}

/**
 * @brief Read inputs from other SWCs
 */
static void FLM_ReadInputs(void) {
    /* Get light switch status from SwitchEvent SWC */
    FLM_State.lightSwitch = SwitchEvent_GetLightRequest();
    FLM_State.e2eStatus = SwitchEvent_GetE2EStatus();

    if (FLM_State.lightSwitch.isValid) {
        FLM_State.switchSignalStatus = SIGNAL_STATUS_VALID;
    } else if (SwitchEvent_IsTimeoutActive()) {
        FLM_State.switchSignalStatus = SIGNAL_STATUS_TIMEOUT;
    } else {
        FLM_State.switchSignalStatus = SIGNAL_STATUS_INVALID;
    }

    /* Get ambient light from LightRequest SWC */
    FLM_State.ambientLight = LightRequest_GetAmbientLight();
    FLM_State.ambientSignalStatus = LightRequest_GetSignalStatus();
}

/**
 * @brief Process state machine
 * @details [FunSafReq01-03] State machine for safe state transition
 */
static void FLM_ProcessStateMachine(void) {
    FLM_State.previousState = FLM_State.currentState;

    switch (FLM_State.currentState) {
        case FLM_STATE_INIT:
            FLM_StateInit();
            break;

        case FLM_STATE_NORMAL:
            FLM_StateNormal();
            break;

        case FLM_STATE_DEGRADED:
            FLM_StateDegraded();
            break;

        case FLM_STATE_SAFE:
            FLM_StateSafe();
            break;

        default:
            /* Invalid state - transition to SAFE */
            FLM_State.currentState = FLM_STATE_SAFE;
            break;
    }

    /* Record state entry time on transition */
    if (FLM_State.currentState != FLM_State.previousState) {
        FLM_State.stateEntryTime = FLM_State.currentTime;
    }
}

/**
 * @brief INIT state processing
 */
static void FLM_StateInit(void) {
    /* Check for external safe state trigger */
    if (FLM_ExternalSafeStateTrigger) {
        FLM_State.currentState = FLM_STATE_SAFE;
        return;
    }

    /* Transition to NORMAL when all inputs are valid */
    if (FLM_AreAllInputsValid()) {
        FLM_State.currentState = FLM_STATE_NORMAL;
        FLM_State.consecutiveErrors = 0U;
    }

    /* Default command in INIT is OFF */
    FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
}

/**
 * @brief NORMAL state processing
 */
static void FLM_StateNormal(void) {
    /* Check for external safe state trigger */
    if (FLM_ExternalSafeStateTrigger) {
        FLM_State.currentState = FLM_STATE_SAFE;
        return;
    }

    /* Check for critical faults -> SAFE state */
    if (FLM_IsCriticalFault()) {
        FLM_State.currentState = FLM_STATE_SAFE;
        return;
    }

    /* Check for any input invalid -> DEGRADED state */
    if (FLM_IsAnyInputInvalid()) {
        FLM_State.consecutiveErrors++;

        if (FLM_State.consecutiveErrors >= FLM_MAX_CONSECUTIVE_ERRORS) {
            FLM_State.currentState = FLM_STATE_DEGRADED;
            FLM_State.degradedEntryTime = FLM_State.currentTime;
        }
    } else {
        FLM_State.consecutiveErrors = 0U;
    }
}

/**
 * @brief DEGRADED state processing
 */
static void FLM_StateDegraded(void) {
    uint32_t timeInDegraded;

    /* Check for external safe state trigger */
    if (FLM_ExternalSafeStateTrigger) {
        FLM_State.currentState = FLM_STATE_SAFE;
        return;
    }

    /* Check if all inputs recovered -> back to NORMAL */
    if (FLM_AreAllInputsValid()) {
        FLM_State.currentState = FLM_STATE_NORMAL;
        FLM_State.consecutiveErrors = 0U;
        return;
    }

    /* Check for critical faults -> SAFE state */
    if (FLM_IsCriticalFault()) {
        FLM_State.currentState = FLM_STATE_SAFE;
        return;
    }

    /* Check degraded timeout [ECU17] FTTI */
    timeInDegraded = FLM_State.currentTime - FLM_State.degradedEntryTime;
    if (timeInDegraded > (FLM_FTTI_MS - FLM_SAFE_STATE_TRANSITION_MS)) {
        FLM_State.currentState = FLM_STATE_SAFE;
    }
}

/**
 * @brief SAFE state processing
 * @details [FunSafReq01-03] Safe state behavior
 */
static void FLM_StateSafe(void) {
    /* Safe state is final - no automatic recovery */
    /* Safe state command is determined based on ambient light */

    /* In safe state, determine headlight based on ambient conditions */
    /* Day (bright): Lights OFF */
    /* Night (dark): Lights ON (low beam) */

    if (FLM_State.ambientLight.isValid) {
        if (FLM_State.ambientLight.adcValue < FLM_AMBIENT_THRESHOLD_ON) {
            /* Dark - turn on low beam for safety */
            FLM_State.headlightCommand = HEADLIGHT_CMD_LOW_BEAM;
        } else {
            /* Bright - turn off */
            FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
        }
    } else {
        /* If ambient sensor also failed, default to LOW_BEAM for safety */
        FLM_State.headlightCommand = HEADLIGHT_CMD_LOW_BEAM;
    }
}

/**
 * @brief Determine headlight command based on current state
 */
static void FLM_DetermineHeadlightCommand(void) {
    /* Safe state has its own logic */
    if (FLM_State.currentState == FLM_STATE_SAFE) {
        return;  /* Already handled in FLM_StateSafe */
    }

    /* INIT state - lights OFF */
    if (FLM_State.currentState == FLM_STATE_INIT) {
        FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
        return;
    }

    /* NORMAL or DEGRADED state - process switch command */
    switch (FLM_State.lightSwitch.command) {
        case LIGHT_SWITCH_OFF:
            FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
            FLM_State.lightsCurrentlyOn = FALSE;
            break;

        case LIGHT_SWITCH_LOW_BEAM:
            FLM_State.headlightCommand = HEADLIGHT_CMD_LOW_BEAM;
            FLM_State.lightsCurrentlyOn = TRUE;
            break;

        case LIGHT_SWITCH_HIGH_BEAM:
            FLM_State.headlightCommand = HEADLIGHT_CMD_HIGH_BEAM;
            FLM_State.lightsCurrentlyOn = TRUE;
            break;

        case LIGHT_SWITCH_AUTO:
            /* AUTO mode - use ambient light */
            FLM_ApplyAutoMode();
            break;

        default:
            /* Invalid command - maintain previous state */
            break;
    }

    /* In degraded mode, if switch is invalid, use AUTO mode logic */
    if ((FLM_State.currentState == FLM_STATE_DEGRADED) &&
        (!FLM_State.lightSwitch.isValid)) {
        FLM_ApplyAutoMode();
    }
}

/**
 * @brief Apply AUTO mode logic with hysteresis
 * @details Hysteresis: ON threshold 800, OFF threshold 1000
 */
static void FLM_ApplyAutoMode(void) {
    if (!FLM_State.ambientLight.isValid) {
        /* If ambient sensor failed in AUTO mode, maintain current state */
        return;
    }

    if (FLM_State.lightsCurrentlyOn) {
        /* Lights are ON - check if should turn OFF (hysteresis) */
        if (FLM_State.ambientLight.adcValue > FLM_AMBIENT_THRESHOLD_OFF) {
            FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
            FLM_State.lightsCurrentlyOn = FALSE;
            FLM_State.hysteresisActive = FALSE;
        } else {
            FLM_State.headlightCommand = HEADLIGHT_CMD_LOW_BEAM;
            FLM_State.hysteresisActive = TRUE;
        }
    } else {
        /* Lights are OFF - check if should turn ON */
        if (FLM_State.ambientLight.adcValue < FLM_AMBIENT_THRESHOLD_ON) {
            FLM_State.headlightCommand = HEADLIGHT_CMD_LOW_BEAM;
            FLM_State.lightsCurrentlyOn = TRUE;
            FLM_State.hysteresisActive = TRUE;
        } else {
            FLM_State.headlightCommand = HEADLIGHT_CMD_OFF;
            FLM_State.hysteresisActive = FALSE;
        }
    }
}

/**
 * @brief Check if all inputs are valid
 */
static boolean FLM_AreAllInputsValid(void) {
    return (FLM_State.lightSwitch.isValid &&
            FLM_State.ambientLight.isValid);
}

/**
 * @brief Check if any input is invalid
 */
static boolean FLM_IsAnyInputInvalid(void) {
    return (!FLM_State.lightSwitch.isValid ||
            !FLM_State.ambientLight.isValid);
}

/**
 * @brief Check for critical faults requiring immediate safe state
 */
static boolean FLM_IsCriticalFault(void) {
    /* E2E timeout exceeds limit */
    if (FLM_State.e2eTimeoutActive) {
        return TRUE;
    }

    /* External safe state request */
    if (FLM_ExternalSafeStateTrigger) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Report checkpoint to Watchdog Manager
 */
static void FLM_ReportWdgMCheckpoint(void) {
    (void)Rte_Call_FLM_WdgM_CheckpointReached(
        FLM_SE_ID,
        FLM_CP_MAIN_ENTRY
    );

    /* Report state machine checkpoint */
    (void)Rte_Call_FLM_WdgM_CheckpointReached(
        FLM_SE_ID,
        FLM_CP_STATE_MACHINE
    );
}

/**
 * @brief Report events to DEM
 */
static void FLM_ReportDemEvents(void) {
    /* Report safe state entry */
    if (FLM_State.currentState == FLM_STATE_SAFE) {
        (void)Rte_Call_FLM_Dem_SetEventStatus(
            DEM_EVENT_SAFE_STATE_ENTERED,
            DEM_EVENT_STATUS_FAILED
        );
    }
}

/*============================================================================*
 * PUBLIC INTERFACE FUNCTIONS
 *============================================================================*/

HeadlightCommand FLM_GetHeadlightCommand(void) {
    return FLM_State.headlightCommand;
}

FLM_StateType FLM_GetCurrentState(void) {
    return FLM_State.currentState;
}

void FLM_TriggerSafeState(SafeStateReason reason) {
    FLM_ExternalSafeStateTrigger = TRUE;
    FLM_SafeStateReason = reason;
}

boolean FLM_IsInSafeState(void) {
    return (FLM_State.currentState == FLM_STATE_SAFE);
}

const FLM_Application_StateType* FLM_GetState(void) {
    return &FLM_State;
}

/*============================================================================*
 * RTE PORT IMPLEMENTATIONS (STUBS)
 *============================================================================*/

Rte_StatusType Rte_Read_FLM_LightSwitchStatus(LightSwitchStatus* status) {
    if (status != NULL_PTR) {
        *status = FLM_State.lightSwitch;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_FLM_E2EStatus(E2E_P01CheckStatusType* e2eStatus) {
    if (e2eStatus != NULL_PTR) {
        *e2eStatus = FLM_State.e2eStatus;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_FLM_AmbientLightLevel(AmbientLightLevel* level) {
    if (level != NULL_PTR) {
        *level = FLM_State.ambientLight;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_FLM_AmbientSignalStatus(SignalStatus* status) {
    if (status != NULL_PTR) {
        *status = FLM_State.ambientSignalStatus;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Write_FLM_HeadlightCommand(HeadlightCommand command) {
    STD_UNUSED(command);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_FLM_CurrentState(FLM_StateType state) {
    STD_UNUSED(state);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_FLM_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    STD_UNUSED(SEId);
    STD_UNUSED(CPId);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_FLM_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
) {
    STD_UNUSED(eventId);
    STD_UNUSED(eventStatus);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_FLM_SafetyMonitor_RequestSafeState(
    SafeStateReason reason
) {
    STD_UNUSED(reason);
    return RTE_E_OK;
}

Rte_StatusType Rte_Mode_FLM_SafetyMode(SafetyStatusType* status) {
    if (status != NULL_PTR) {
        *status = SAFETY_STATUS_OK;
    }
    return RTE_E_OK;
}

Rte_TimestampType Rte_IrvRead_FLM_SystemTime(void) {
    return FLM_SystemTime;
}
