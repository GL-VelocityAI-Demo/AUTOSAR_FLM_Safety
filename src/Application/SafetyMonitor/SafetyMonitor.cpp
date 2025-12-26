/**
 * @file SafetyMonitor.cpp
 * @brief SafetyMonitor SWC Implementation
 * @details Aggregates safety status and triggers safe state
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [ECU17] FTTI monitoring, [FunSafReq01-03] Safe state control
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "SafetyMonitor.h"
#include "Application/SwitchEvent/SwitchEvent.h"
#include "Application/LightRequest/LightRequest.h"
#include "Application/FLM/FLM_Application.h"
#include "Application/Headlight/Headlight.h"
#include "Dem_Cfg.h"
#include <cstring>

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Component internal state */
static SafetyMonitor_StateType SafetyMonitor_State;

/** @brief System time counter */
static uint32_t SafetyMonitor_SystemTime = 0U;

/** @brief Simulated WdgM status for testing */
static WdgM_GlobalStatusType SafetyMonitor_SimWdgmStatus = WDGM_GLOBAL_STATUS_OK;
static boolean SafetyMonitor_SimWdgmEnabled = FALSE;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void SafetyMonitor_ReadComponentStatus(void);
static void SafetyMonitor_AggregrateFaults(void);
static void SafetyMonitor_CheckE2ETimeout(void);
static void SafetyMonitor_CheckWdgMStatus(void);
static void SafetyMonitor_CheckFTTI(void);
static void SafetyMonitor_UpdateGlobalStatus(void);
static void SafetyMonitor_DetermineSafeStateCommand(void);
static void SafetyMonitor_ReportDemEvents(void);
static void SafetyMonitor_ReportWdgMCheckpoint(void);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize SafetyMonitor component
 */
void SafetyMonitor_Init(void) {
    /* Clear state structure */
    (void)memset(&SafetyMonitor_State, 0, sizeof(SafetyMonitor_State));

    /* Initialize safe state */
    SafetyMonitor_State.inSafeState = FALSE;
    SafetyMonitor_State.safeStateReason = SAFE_STATE_REASON_NONE;
    SafetyMonitor_State.safeStateCommand = HEADLIGHT_CMD_OFF;

    /* Initialize fault tracking */
    SafetyMonitor_State.totalFaultCount = 0U;
    SafetyMonitor_State.fttiActive = FALSE;

    /* Initialize global status */
    SafetyMonitor_State.globalStatus = SAFETY_STATUS_OK;
    SafetyMonitor_State.wdgmGlobalStatus = WDGM_GLOBAL_STATUS_OK;

    /* Initialize ambient light tracking */
    SafetyMonitor_State.isDaytime = TRUE;  /* Assume daytime initially */

    /* Mark as initialized */
    SafetyMonitor_State.isInitialized = TRUE;
}

/**
 * @brief Main function for SafetyMonitor
 * @details [ECU17] FTTI monitoring (200ms)
 */
void SafetyMonitor_MainFunction(void) {
    if (!SafetyMonitor_State.isInitialized) {
        return;
    }

    /* Report entry checkpoint to WdgM */
    SafetyMonitor_ReportWdgMCheckpoint();

    /* Update timestamp */
    SafetyMonitor_State.currentTime = SafetyMonitor_SystemTime;
    SafetyMonitor_SystemTime += FLM_SAFETY_MONITOR_PERIOD_MS;

    /* Read status from all components */
    SafetyMonitor_ReadComponentStatus();

    /* Aggregate faults */
    SafetyMonitor_AggregrateFaults();

    /* Check E2E timeout [SysSafReq02] */
    SafetyMonitor_CheckE2ETimeout();

    /* Check WdgM status [SysSafReq03] */
    SafetyMonitor_CheckWdgMStatus();

    /* Check FTTI [ECU17] */
    SafetyMonitor_CheckFTTI();

    /* Update global safety status */
    SafetyMonitor_UpdateGlobalStatus();

    /* If in safe state, determine safe command */
    if (SafetyMonitor_State.inSafeState) {
        SafetyMonitor_DetermineSafeStateCommand();
    }

    /* Report DEM events */
    SafetyMonitor_ReportDemEvents();
}

/**
 * @brief Read status from all monitored components
 */
static void SafetyMonitor_ReadComponentStatus(void) {
    LightSwitchStatus switchStatus;
    AmbientLightLevel ambientLevel;

    /* Get SwitchEvent status */
    switchStatus = SwitchEvent_GetLightRequest();
    SafetyMonitor_State.switchEventFault = !switchStatus.isValid;
    SafetyMonitor_State.e2eStatus = SwitchEvent_GetE2EStatus();
    SafetyMonitor_State.e2eSmStatus = SwitchEvent_GetE2ESmStatus();

    /* Get LightRequest status */
    ambientLevel = LightRequest_GetAmbientLight();
    SafetyMonitor_State.lastAmbientLight = ambientLevel;
    SafetyMonitor_State.lightRequestFault = !ambientLevel.isValid;

    /* Determine if it's daytime based on ambient light */
    if (ambientLevel.isValid) {
        SafetyMonitor_State.isDaytime =
            (ambientLevel.adcValue > SAFETYMONITOR_DAY_THRESHOLD);
    }

    /* Get FLM status */
    SafetyMonitor_State.flmState = FLM_GetCurrentState();
    SafetyMonitor_State.flmFault =
        (SafetyMonitor_State.flmState == FLM_STATE_SAFE);

    /* Get Headlight status */
    SafetyMonitor_State.headlightStatus = Headlight_GetFaultStatus();
    SafetyMonitor_State.headlightFault =
        (SafetyMonitor_State.headlightStatus != HEADLIGHT_FAULT_NONE);

    /* Get WdgM status */
    if (SafetyMonitor_SimWdgmEnabled) {
        SafetyMonitor_State.wdgmGlobalStatus = SafetyMonitor_SimWdgmStatus;
    } else {
        /* In real system, would call WdgM_GetGlobalStatus */
        SafetyMonitor_State.wdgmGlobalStatus = WDGM_GLOBAL_STATUS_OK;
    }

    SafetyMonitor_State.wdgmFault =
        (SafetyMonitor_State.wdgmGlobalStatus != WDGM_GLOBAL_STATUS_OK);
}

/**
 * @brief Aggregate faults from all components
 */
static void SafetyMonitor_AggregrateFaults(void) {
    uint8_t faultCount = 0U;

    if (SafetyMonitor_State.switchEventFault) {
        faultCount++;
    }
    if (SafetyMonitor_State.lightRequestFault) {
        faultCount++;
    }
    if (SafetyMonitor_State.headlightFault) {
        faultCount++;
    }
    if (SafetyMonitor_State.wdgmFault) {
        faultCount++;
    }

    /* Track first fault time */
    if ((faultCount > 0U) && (SafetyMonitor_State.totalFaultCount == 0U)) {
        SafetyMonitor_State.firstFaultTime = SafetyMonitor_State.currentTime;
        SafetyMonitor_State.fttiActive = TRUE;
    }

    /* Clear FTTI if no faults */
    if (faultCount == 0U) {
        SafetyMonitor_State.fttiActive = FALSE;
    }

    SafetyMonitor_State.totalFaultCount = faultCount;

    /* Multiple faults trigger immediate safe state */
    if (faultCount >= SAFETYMONITOR_MAX_FAULT_COUNT) {
        SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_MULTI_FAULT);
    }
}

/**
 * @brief Check E2E timeout
 * @details [SysSafReq02] E2E protection timeout (100ms)
 */
static void SafetyMonitor_CheckE2ETimeout(void) {
    /* Check if E2E state machine is not valid */
    if (SafetyMonitor_State.e2eSmStatus != E2E_SM_VALID) {
        if (!SafetyMonitor_State.e2eTimeoutActive) {
            /* Start timeout tracking */
            SafetyMonitor_State.e2eFailureStartTime =
                SafetyMonitor_State.currentTime;
            SafetyMonitor_State.e2eTimeoutActive = TRUE;
        } else {
            /* Check if timeout exceeded */
            uint32_t elapsed = SafetyMonitor_State.currentTime -
                               SafetyMonitor_State.e2eFailureStartTime;

            if (elapsed >= SAFETYMONITOR_E2E_TIMEOUT_MS) {
                SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_E2E_FAILURE);
            }
        }
    } else {
        /* E2E is valid, reset timeout */
        SafetyMonitor_State.e2eTimeoutActive = FALSE;
    }
}

/**
 * @brief Check WdgM status
 * @details [SysSafReq03] Watchdog supervision
 */
static void SafetyMonitor_CheckWdgMStatus(void) {
    /* WdgM failure triggers immediate safe state */
    if ((SafetyMonitor_State.wdgmGlobalStatus == WDGM_GLOBAL_STATUS_FAILED) ||
        (SafetyMonitor_State.wdgmGlobalStatus == WDGM_GLOBAL_STATUS_EXPIRED)) {
        SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_WDGM_FAILURE);
    }
}

/**
 * @brief Check Fault Tolerant Time Interval
 * @details [ECU17] FTTI = 200ms
 */
static void SafetyMonitor_CheckFTTI(void) {
    uint32_t elapsed;

    if (!SafetyMonitor_State.fttiActive) {
        return;
    }

    elapsed = SafetyMonitor_State.currentTime -
              SafetyMonitor_State.firstFaultTime;

    /* If faults persist beyond FTTI, trigger safe state */
    if (elapsed >= SAFETYMONITOR_FTTI_MS) {
        SafetyMonitor_TriggerSafeState(SAFE_STATE_REASON_TIMEOUT);
    }
}

/**
 * @brief Update global safety status
 */
static void SafetyMonitor_UpdateGlobalStatus(void) {
    if (SafetyMonitor_State.inSafeState) {
        SafetyMonitor_State.globalStatus = SAFETY_STATUS_SAFE_STATE;
    } else if (SafetyMonitor_State.totalFaultCount > 0U) {
        if (SafetyMonitor_State.totalFaultCount >= 2U) {
            SafetyMonitor_State.globalStatus = SAFETY_STATUS_DEGRADED;
        } else {
            SafetyMonitor_State.globalStatus = SAFETY_STATUS_WARNING;
        }
    } else {
        SafetyMonitor_State.globalStatus = SAFETY_STATUS_OK;
    }
}

/**
 * @brief Determine safe state headlight command
 * @details Day: Lights OFF, Night: Lights ON (low beam)
 */
static void SafetyMonitor_DetermineSafeStateCommand(void) {
    if (SafetyMonitor_State.isDaytime) {
        /* Day - lights OFF is safe */
        SafetyMonitor_State.safeStateCommand = HEADLIGHT_CMD_OFF;
    } else {
        /* Night - low beam ON for visibility */
        SafetyMonitor_State.safeStateCommand = HEADLIGHT_CMD_LOW_BEAM;
    }
}

/**
 * @brief Report checkpoint to Watchdog Manager
 */
static void SafetyMonitor_ReportWdgMCheckpoint(void) {
    (void)Rte_Call_SafetyMonitor_WdgM_CheckpointReached(
        SAFETYMONITOR_SE_ID,
        SAFETYMONITOR_CP_MAIN_ENTRY
    );

    /* Report aggregation checkpoint */
    (void)Rte_Call_SafetyMonitor_WdgM_CheckpointReached(
        SAFETYMONITOR_SE_ID,
        SAFETYMONITOR_CP_AGGREGATION
    );
}

/**
 * @brief Report events to DEM
 */
static void SafetyMonitor_ReportDemEvents(void) {
    /* Report WdgM failure */
    if (SafetyMonitor_State.wdgmFault) {
        (void)Rte_Call_SafetyMonitor_Dem_SetEventStatus(
            DEM_EVENT_WDGM_SUPERVISION_FAILED,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_SafetyMonitor_Dem_SetEventStatus(
            DEM_EVENT_WDGM_SUPERVISION_FAILED,
            DEM_EVENT_STATUS_PASSED
        );
    }

    /* Report safe state entry */
    if (SafetyMonitor_State.inSafeState) {
        (void)Rte_Call_SafetyMonitor_Dem_SetEventStatus(
            DEM_EVENT_SAFE_STATE_ENTERED,
            DEM_EVENT_STATUS_FAILED
        );
    }
}

/*============================================================================*
 * PUBLIC INTERFACE FUNCTIONS
 *============================================================================*/

SafetyStatusType SafetyMonitor_GetGlobalStatus(void) {
    return SafetyMonitor_State.globalStatus;
}

void SafetyMonitor_TriggerSafeState(SafeStateReason reason) {
    if (!SafetyMonitor_State.inSafeState) {
        SafetyMonitor_State.inSafeState = TRUE;
        SafetyMonitor_State.safeStateReason = reason;
        SafetyMonitor_State.safeStateEntryTime = SafetyMonitor_State.currentTime;
        SafetyMonitor_State.globalStatus = SAFETY_STATUS_SAFE_STATE;

        /* Trigger FLM to enter safe state */
        FLM_TriggerSafeState(reason);
    }
}

boolean SafetyMonitor_IsInSafeState(void) {
    return SafetyMonitor_State.inSafeState;
}

SafeStateReason SafetyMonitor_GetSafeStateReason(void) {
    return SafetyMonitor_State.safeStateReason;
}

HeadlightCommand SafetyMonitor_GetSafeStateCommand(void) {
    return SafetyMonitor_State.safeStateCommand;
}

const SafetyMonitor_StateType* SafetyMonitor_GetState(void) {
    return &SafetyMonitor_State;
}

/*============================================================================*
 * TEST HELPER FUNCTIONS
 *============================================================================*/

/**
 * @brief Set simulated WdgM status (for testing)
 * @param[in] status WdgM status to simulate
 */
void SafetyMonitor_SimSetWdgmStatus(WdgM_GlobalStatusType status) {
    SafetyMonitor_SimWdgmStatus = status;
    SafetyMonitor_SimWdgmEnabled = TRUE;
}

/*============================================================================*
 * RTE PORT IMPLEMENTATIONS (STUBS)
 *============================================================================*/

Rte_StatusType Rte_Read_SafetyMonitor_FLMState(FLM_StateType* state) {
    if (state != NULL_PTR) {
        *state = SafetyMonitor_State.flmState;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_SafetyMonitor_E2EStatus(E2E_P01CheckStatusType* status) {
    if (status != NULL_PTR) {
        *status = SafetyMonitor_State.e2eStatus;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_SafetyMonitor_HeadlightFault(HeadlightFaultStatus* status) {
    if (status != NULL_PTR) {
        *status = SafetyMonitor_State.headlightStatus;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_SafetyMonitor_AmbientStatus(SignalStatus* status) {
    if (status != NULL_PTR) {
        *status = SafetyMonitor_State.lastAmbientLight.isValid ?
                  SIGNAL_STATUS_VALID : SIGNAL_STATUS_INVALID;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Read_SafetyMonitor_AmbientLight(AmbientLightLevel* level) {
    if (level != NULL_PTR) {
        *level = SafetyMonitor_State.lastAmbientLight;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Write_SafetyMonitor_GlobalStatus(SafetyStatusType status) {
    STD_UNUSED(status);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_SafetyMonitor_SafeStateActive(boolean active) {
    STD_UNUSED(active);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_SafetyMonitor_SafeStateCommand(HeadlightCommand command) {
    STD_UNUSED(command);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_SafetyMonitor_WdgM_GetGlobalStatus(
    WdgM_GlobalStatusType* status
) {
    if (status != NULL_PTR) {
        *status = SafetyMonitor_State.wdgmGlobalStatus;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Call_SafetyMonitor_WdgM_GetLocalStatus(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_LocalStatusType* status
) {
    STD_UNUSED(SEId);
    if (status != NULL_PTR) {
        *status = WDGM_LOCAL_STATUS_OK;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Call_SafetyMonitor_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    STD_UNUSED(SEId);
    STD_UNUSED(CPId);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_SafetyMonitor_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
) {
    STD_UNUSED(eventId);
    STD_UNUSED(eventStatus);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_SafetyMonitor_BswM_RequestReset(void) {
    return RTE_E_OK;
}

Rte_TimestampType Rte_IrvRead_SafetyMonitor_SystemTime(void) {
    return SafetyMonitor_SystemTime;
}
