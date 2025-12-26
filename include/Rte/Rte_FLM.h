/**
 * @file Rte_FLM.h
 * @brief RTE Interface for FLM Application SWC
 * @details Provides RTE API for the FLM Application software component (ASIL B)
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - This component is safety-relevant
 */

#ifndef RTE_FLM_H
#define RTE_FLM_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte_Type.h"

/*============================================================================*
 * COMPONENT DATA STRUCTURES
 *============================================================================*/

/**
 * @brief FLM Application Component Data Structure
 */
typedef struct {
    /* State machine */
    FLM_StateType currentState;
    FLM_StateType previousState;

    /* Input data */
    LightSwitchStatus lightSwitch;
    AmbientLightLevel ambientLight;
    SignalStatus switchSignalStatus;
    SignalStatus ambientSignalStatus;
    E2E_P01CheckStatusType e2eStatus;

    /* Output data */
    HeadlightCommand headlightCommand;

    /* Timing */
    Rte_TimestampType stateEntryTime;
    Rte_TimestampType currentTime;
    Rte_TimestampType degradedEntryTime;

    /* Hysteresis state */
    boolean lightsWereOn;

    /* Error tracking */
    uint8_t consecutiveErrors;
    boolean e2eTimeoutActive;
    Rte_TimestampType e2eTimeoutStart;

    /* Initialization flag */
    boolean isInitialized;
} Rte_FLM_DataType;

/*============================================================================*
 * RTE API - SENDER/RECEIVER PORTS
 *============================================================================*/

/**
 * @brief Read light switch status from SwitchEvent SWC
 * @param[out] status Pointer to receive light switch status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_FLM_LightSwitchStatus(LightSwitchStatus* status);

/**
 * @brief Read E2E status from SwitchEvent SWC
 * @param[out] e2eStatus Pointer to receive E2E status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_FLM_E2EStatus(E2E_P01CheckStatusType* e2eStatus);

/**
 * @brief Read ambient light level from LightRequest SWC
 * @param[out] level Pointer to receive ambient light level
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_FLM_AmbientLightLevel(AmbientLightLevel* level);

/**
 * @brief Read ambient light signal status
 * @param[out] status Pointer to receive signal status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_FLM_AmbientSignalStatus(SignalStatus* status);

/**
 * @brief Write headlight command to Headlight SWC
 * @param[in] command Headlight command to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_FLM_HeadlightCommand(HeadlightCommand command);

/**
 * @brief Write current FLM state
 * @param[in] state Current state
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_FLM_CurrentState(FLM_StateType state);

/*============================================================================*
 * RTE API - CLIENT/SERVER PORTS
 *============================================================================*/

/**
 * @brief Report checkpoint to Watchdog Manager
 * @details [SysSafReq03] Logical supervision checkpoint
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_FLM_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Report event to Diagnostic Event Manager
 * @param[in] eventId Event identifier
 * @param[in] eventStatus Event status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_FLM_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
);

/**
 * @brief Request safe state from Safety Monitor
 * @param[in] reason Reason for safe state request
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_FLM_SafetyMonitor_RequestSafeState(
    SafeStateReason reason
);

/*============================================================================*
 * RTE API - MODE PORTS
 *============================================================================*/

/**
 * @brief Get current safety mode
 * @param[out] status Current safety status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Mode_FLM_SafetyMode(SafetyStatusType* status);

/*============================================================================*
 * RTE API - INTERNAL (IRV)
 *============================================================================*/

/**
 * @brief Get current system time
 * @return Current timestamp in milliseconds
 */
Rte_TimestampType Rte_IrvRead_FLM_SystemTime(void);

/*============================================================================*
 * RUNNABLE DECLARATIONS
 *============================================================================*/

/**
 * @brief Initialization runnable
 * @details Called once during ECU startup
 */
void FLM_Init(void);

/**
 * @brief Main function runnable
 * @details Called periodically every 10ms
 *          Implements main control logic
 */
void FLM_MainFunction(void);

/**
 * @brief Get current headlight command
 * @return Current headlight command
 */
HeadlightCommand FLM_GetHeadlightCommand(void);

/**
 * @brief Get current FLM state
 * @return Current state machine state
 */
FLM_StateType FLM_GetCurrentState(void);

/**
 * @brief Trigger transition to safe state
 * @param[in] reason Reason for safe state transition
 */
void FLM_TriggerSafeState(SafeStateReason reason);

/*============================================================================*
 * SUPERVISION ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Supervision Entity ID for FLM */
#define FLM_SE_ID                   0x0003U

/** @brief Checkpoint ID for main function entry */
#define FLM_CP_MAIN_ENTRY           0x0001U

/** @brief Checkpoint ID for state machine execution */
#define FLM_CP_STATE_MACHINE        0x0002U

/** @brief Checkpoint ID for main function exit */
#define FLM_CP_MAIN_EXIT            0x0003U

/*============================================================================*
 * DEM EVENT CONFIGURATION
 *============================================================================*/

/** @brief State machine error event ID */
#define FLM_DEM_STATE_ERROR         0x0010U

/** @brief Safe state entered event ID */
#define FLM_DEM_SAFE_STATE          0x0009U

#endif /* RTE_FLM_H */
