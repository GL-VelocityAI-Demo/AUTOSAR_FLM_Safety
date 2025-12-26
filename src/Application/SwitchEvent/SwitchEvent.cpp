/**
 * @file SwitchEvent.cpp
 * @brief SwitchEvent SWC Implementation
 * @details Receives CAN light switch signals and validates them with E2E protection
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [FunSafReq01-01] Light request plausibility check
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "SwitchEvent.h"
#include "Dem_Cfg.h"
#include <cstring>

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Component internal state */
static SwitchEvent_StateType SwitchEvent_State;

/** @brief System time counter (simulated, incremented each main function call) */
static uint32_t SwitchEvent_SystemTime = 0U;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void SwitchEvent_InitE2E(void);
static void SwitchEvent_PerformE2ECheck(void);
static void SwitchEvent_UpdateTimeoutStatus(void);
static void SwitchEvent_ExtractLightSwitchCommand(const uint8_t* data);
static void SwitchEvent_ReportWdgMCheckpoint(void);
static void SwitchEvent_ReportDemEvents(void);
static boolean SwitchEvent_IsCommandValid(uint8_t command);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize SwitchEvent component
 */
void SwitchEvent_Init(void) {
    /* Clear state structure */
    (void)memset(&SwitchEvent_State, 0, sizeof(SwitchEvent_State));

    /* Initialize E2E protection */
    SwitchEvent_InitE2E();

    /* Initialize light switch status */
    SwitchEvent_State.lightSwitchStatus.command = LIGHT_SWITCH_OFF;
    SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
    SwitchEvent_State.lightSwitchStatus.e2eStatus = 0U;

    /* Initialize timing */
    SwitchEvent_State.timeoutCounter = 0U;
    SwitchEvent_State.e2eTimeoutCounter = 0U;
    SwitchEvent_State.lastValidTimestamp = 0U;
    SwitchEvent_State.currentTimestamp = 0U;

    /* Initialize error tracking */
    SwitchEvent_State.consecutiveE2EErrors = 0U;
    SwitchEvent_State.consecutiveTimeouts = 0U;
    SwitchEvent_State.e2eFailureActive = FALSE;
    SwitchEvent_State.timeoutActive = FALSE;

    /* No new message initially */
    SwitchEvent_State.newMessageReceived = FALSE;

    /* Mark as initialized */
    SwitchEvent_State.isInitialized = TRUE;
}

/**
 * @brief Initialize E2E protection
 */
static void SwitchEvent_InitE2E(void) {
    /* Configure E2E Profile 01 for light switch message */
    SwitchEvent_State.e2eConfig.DataLength = FLM_E2E_LIGHTSWITCH_DATA_LENGTH;
    SwitchEvent_State.e2eConfig.DataID = FLM_E2E_LIGHTSWITCH_DATA_ID;
    SwitchEvent_State.e2eConfig.MaxDeltaCounter = FLM_E2E_MAX_DELTA_COUNTER;
    SwitchEvent_State.e2eConfig.MaxNoNewOrRepeatedData = FLM_E2E_MAX_NO_NEW_DATA;
    SwitchEvent_State.e2eConfig.SyncCounter = FLM_E2E_SYNC_COUNTER;
    SwitchEvent_State.e2eConfig.CounterOffset = FLM_E2E_COUNTER_OFFSET;
    SwitchEvent_State.e2eConfig.CRCOffset = FLM_E2E_CRC_OFFSET;
    SwitchEvent_State.e2eConfig.DataIDNibbleOffset = 0U;
    SwitchEvent_State.e2eConfig.DataIDMode = FALSE;

    /* Initialize E2E check state */
    (void)E2E_P01CheckInit(&SwitchEvent_State.e2eCheckState);

    /* Configure E2E state machine */
    SwitchEvent_State.e2eSmConfig.WindowSize = 5U;
    SwitchEvent_State.e2eSmConfig.MinOkStateInit = 2U;
    SwitchEvent_State.e2eSmConfig.MaxErrorStateInit = 2U;
    SwitchEvent_State.e2eSmConfig.MinOkStateValid = 2U;
    SwitchEvent_State.e2eSmConfig.MinOkStateInvalid = 3U;
    SwitchEvent_State.e2eSmConfig.MaxErrorStateValid = 2U;
    SwitchEvent_State.e2eSmConfig.MaxErrorStateInvalid = 3U;

    /* Initialize E2E state machine */
    (void)E2E_SMCheckInit(&SwitchEvent_State.e2eSmState);
}

/**
 * @brief Main function for SwitchEvent
 * @details [FunSafReq01-01] Performs E2E check and validates light switch request
 */
void SwitchEvent_MainFunction(void) {
    if (!SwitchEvent_State.isInitialized) {
        return;
    }

    /* Report entry checkpoint to WdgM [SysSafReq03] */
    SwitchEvent_ReportWdgMCheckpoint();

    /* Update current timestamp */
    SwitchEvent_State.currentTimestamp = SwitchEvent_SystemTime;
    SwitchEvent_SystemTime += FLM_MAIN_FUNCTION_PERIOD_MS;

    /* Perform E2E check on received data [SysSafReq02] */
    SwitchEvent_PerformE2ECheck();

    /* Update timeout status [SysSafReq01] */
    SwitchEvent_UpdateTimeoutStatus();

    /* Determine overall validity */
    if (SwitchEvent_State.e2eSmStatus == E2E_SM_VALID) {
        if (!SwitchEvent_State.timeoutActive) {
            SwitchEvent_State.lightSwitchStatus.isValid = TRUE;
        } else {
            SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
        }
    } else {
        SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
    }

    /* Update E2E status in light switch structure */
    SwitchEvent_State.lightSwitchStatus.e2eStatus =
        static_cast<uint8_t>(SwitchEvent_State.e2eStatus);

    /* Report DEM events */
    SwitchEvent_ReportDemEvents();
}

/**
 * @brief Perform E2E check on received message
 * @details [SysSafReq02] E2E protection verification
 */
static void SwitchEvent_PerformE2ECheck(void) {
    if (SwitchEvent_State.newMessageReceived) {
        /* Perform E2E Profile 01 check */
        SwitchEvent_State.e2eStatus = E2E_P01Check(
            &SwitchEvent_State.e2eConfig,
            &SwitchEvent_State.e2eCheckState,
            SwitchEvent_State.lastMessageData,
            FLM_CAN_LIGHTSWITCH_MSG_LEN
        );

        /* Update E2E state machine */
        SwitchEvent_State.e2eSmStatus = E2E_SMCheck(
            &SwitchEvent_State.e2eSmConfig,
            &SwitchEvent_State.e2eSmState,
            SwitchEvent_State.e2eStatus
        );

        /* Process result */
        if ((SwitchEvent_State.e2eStatus == E2E_P01STATUS_OK) ||
            (SwitchEvent_State.e2eStatus == E2E_P01STATUS_OKSOMELOST) ||
            (SwitchEvent_State.e2eStatus == E2E_P01STATUS_INITIAL)) {
            /* Valid E2E, extract command */
            SwitchEvent_ExtractLightSwitchCommand(SwitchEvent_State.lastMessageData);
            SwitchEvent_State.consecutiveE2EErrors = 0U;
            SwitchEvent_State.lastValidTimestamp = SwitchEvent_State.currentTimestamp;
            SwitchEvent_State.timeoutCounter = 0U;
            SwitchEvent_State.e2eFailureActive = FALSE;
        } else {
            /* E2E error */
            SwitchEvent_State.consecutiveE2EErrors++;

            if (SwitchEvent_State.consecutiveE2EErrors >= SWITCHEVENT_MAX_E2E_ERRORS) {
                SwitchEvent_State.e2eFailureActive = TRUE;
                SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
            }
        }

        SwitchEvent_State.newMessageReceived = FALSE;
    } else {
        /* No new data - perform check with NULL to update state machine */
        SwitchEvent_State.e2eStatus = E2E_P01Check(
            &SwitchEvent_State.e2eConfig,
            &SwitchEvent_State.e2eCheckState,
            NULL_PTR,
            0U
        );

        /* Update state machine with no-data status */
        SwitchEvent_State.e2eSmStatus = E2E_SMCheck(
            &SwitchEvent_State.e2eSmConfig,
            &SwitchEvent_State.e2eSmState,
            SwitchEvent_State.e2eStatus
        );
    }
}

/**
 * @brief Update timeout status
 * @details [SysSafReq01] CAN timeout detection (50ms)
 */
static void SwitchEvent_UpdateTimeoutStatus(void) {
    /* Increment timeout counter if no new message */
    if (!SwitchEvent_State.newMessageReceived) {
        SwitchEvent_State.timeoutCounter++;
    }

    /* Check for timeout */
    if (SwitchEvent_State.timeoutCounter >= SWITCHEVENT_TIMEOUT_CYCLES) {
        SwitchEvent_State.timeoutActive = TRUE;
        SwitchEvent_State.consecutiveTimeouts++;
        SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
    } else {
        SwitchEvent_State.timeoutActive = FALSE;
        SwitchEvent_State.consecutiveTimeouts = 0U;
    }

    /* Update E2E timeout counter */
    if (SwitchEvent_State.e2eSmStatus != E2E_SM_VALID) {
        SwitchEvent_State.e2eTimeoutCounter++;

        if (SwitchEvent_State.e2eTimeoutCounter >= SWITCHEVENT_E2E_TIMEOUT_CYCLES) {
            SwitchEvent_State.e2eFailureActive = TRUE;
        }
    } else {
        SwitchEvent_State.e2eTimeoutCounter = 0U;
    }
}

/**
 * @brief Extract light switch command from message data
 * @param[in] data Pointer to message data
 */
static void SwitchEvent_ExtractLightSwitchCommand(const uint8_t* data) {
    uint8_t command;

    if (data == NULL_PTR) {
        return;
    }

    /* Command is in byte 2 of the message */
    command = data[COM_LIGHTSWITCH_CMD_BYTE];

    /* Validate command range [FunSafReq01-01] */
    if (SwitchEvent_IsCommandValid(command)) {
        SwitchEvent_State.lightSwitchStatus.command =
            static_cast<LightSwitchCmd>(command);
    } else {
        /* Invalid command - keep previous valid command, mark as invalid */
        SwitchEvent_State.lightSwitchStatus.isValid = FALSE;
    }
}

/**
 * @brief Check if command value is valid
 * @param[in] command Command value to check
 * @return TRUE if valid
 */
static boolean SwitchEvent_IsCommandValid(uint8_t command) {
    /* Valid commands: 0=OFF, 1=LOW_BEAM, 2=HIGH_BEAM, 3=AUTO */
    return (command <= static_cast<uint8_t>(LIGHT_SWITCH_AUTO));
}

/**
 * @brief Report checkpoint to Watchdog Manager
 * @details [SysSafReq03] Alive supervision
 */
static void SwitchEvent_ReportWdgMCheckpoint(void) {
    /* Report entry checkpoint */
    (void)Rte_Call_SwitchEvent_WdgM_CheckpointReached(
        SWITCHEVENT_SE_ID,
        SWITCHEVENT_CP_MAIN_ENTRY
    );
}

/**
 * @brief Report events to Diagnostic Event Manager
 */
static void SwitchEvent_ReportDemEvents(void) {
    /* Report E2E failure */
    if (SwitchEvent_State.e2eFailureActive) {
        (void)Rte_Call_SwitchEvent_Dem_SetEventStatus(
            DEM_EVENT_E2E_LIGHTSWITCH_FAILED,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_SwitchEvent_Dem_SetEventStatus(
            DEM_EVENT_E2E_LIGHTSWITCH_FAILED,
            DEM_EVENT_STATUS_PASSED
        );
    }

    /* Report CAN timeout */
    if (SwitchEvent_State.timeoutActive) {
        (void)Rte_Call_SwitchEvent_Dem_SetEventStatus(
            DEM_EVENT_CAN_TIMEOUT,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_SwitchEvent_Dem_SetEventStatus(
            DEM_EVENT_CAN_TIMEOUT,
            DEM_EVENT_STATUS_PASSED
        );
    }
}

/*============================================================================*
 * PUBLIC INTERFACE FUNCTIONS
 *============================================================================*/

E2E_P01CheckStatusType SwitchEvent_GetE2EStatus(void) {
    return SwitchEvent_State.e2eStatus;
}

LightSwitchStatus SwitchEvent_GetLightRequest(void) {
    return SwitchEvent_State.lightSwitchStatus;
}

E2E_SMStateType SwitchEvent_GetE2ESmStatus(void) {
    return SwitchEvent_State.e2eSmStatus;
}

boolean SwitchEvent_IsTimeoutActive(void) {
    return SwitchEvent_State.timeoutActive;
}

void SwitchEvent_ProcessCanMessage(const uint8_t* data, uint8_t length) {
    if ((data == NULL_PTR) || (length != FLM_CAN_LIGHTSWITCH_MSG_LEN)) {
        return;
    }

    /* Copy message data */
    (void)memcpy(SwitchEvent_State.lastMessageData, data, length);
    SwitchEvent_State.newMessageReceived = TRUE;
}

const SwitchEvent_StateType* SwitchEvent_GetState(void) {
    return &SwitchEvent_State;
}

/*============================================================================*
 * RTE PORT IMPLEMENTATIONS (STUBS)
 *============================================================================*/

Rte_StatusType Rte_Read_SwitchEvent_CanLightSwitchMsg(Rte_CanMessageType* message) {
    if (message == NULL_PTR) {
        return RTE_E_INVALID;
    }

    if (SwitchEvent_State.newMessageReceived) {
        (void)memcpy(message->data, SwitchEvent_State.lastMessageData,
                     FLM_CAN_LIGHTSWITCH_MSG_LEN);
        message->length = FLM_CAN_LIGHTSWITCH_MSG_LEN;
        message->isNew = TRUE;
        return RTE_E_OK;
    }

    message->isNew = FALSE;
    return RTE_E_NO_DATA;
}

Rte_StatusType Rte_Write_SwitchEvent_LightSwitchStatus(const LightSwitchStatus* status) {
    STD_UNUSED(status);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_SwitchEvent_E2EStatus(E2E_P01CheckStatusType e2eStatus) {
    STD_UNUSED(e2eStatus);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_SwitchEvent_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    STD_UNUSED(SEId);
    STD_UNUSED(CPId);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_SwitchEvent_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
) {
    STD_UNUSED(eventId);
    STD_UNUSED(eventStatus);
    return RTE_E_OK;
}

Rte_StatusType Rte_Mode_SwitchEvent_ComMMode(ComM_ModeType* mode) {
    if (mode != NULL_PTR) {
        *mode = COMM_FULL_COMMUNICATION;
    }
    return RTE_E_OK;
}

Rte_TimestampType Rte_IrvRead_SwitchEvent_SystemTime(void) {
    return SwitchEvent_SystemTime;
}
