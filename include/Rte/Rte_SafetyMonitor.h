/**
 * @file Rte_SafetyMonitor.h
 * @brief RTE Interface for SafetyMonitor SWC
 * @details Provides RTE API for the SafetyMonitor software component (ASIL B)
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - This component is safety-relevant
 */

#ifndef RTE_SAFETYMONITOR_H
#define RTE_SAFETYMONITOR_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte_Type.h"

/*============================================================================*
 * CONFIGURATION CONSTANTS
 *============================================================================*/

/** @brief Fault Tolerant Time Interval in ms [ECU17] */
#define SAFETYMONITOR_FTTI_MS               200U

/** @brief Safe state transition time in ms [FunSafReq01-03] */
#define SAFETYMONITOR_SAFE_STATE_TIME_MS    100U

/** @brief E2E timeout threshold in ms [SysSafReq02] */
#define SAFETYMONITOR_E2E_TIMEOUT_MS        100U

/** @brief Maximum fault count before safe state */
#define SAFETYMONITOR_MAX_FAULT_COUNT       3U

/** @brief Ambient light threshold for day/night detection */
#define SAFETYMONITOR_DAY_THRESHOLD         1500U

/*============================================================================*
 * COMPONENT DATA STRUCTURES
 *============================================================================*/

/**
 * @brief Component fault status structure
 */
typedef struct {
    boolean switchEventFault;
    boolean lightRequestFault;
    boolean flmFault;
    boolean headlightFault;
    boolean wdgmFault;
    E2E_P01CheckStatusType e2eStatus;
    HeadlightFaultStatus headlightStatus;
    FLM_StateType flmState;
} SafetyMonitor_ComponentStatus;

/**
 * @brief SafetyMonitor Component Data Structure
 */
typedef struct {
    /* Internal state */
    boolean isInitialized;
    boolean inSafeState;

    /* Component status */
    SafetyMonitor_ComponentStatus componentStatus;

    /* Fault tracking */
    uint8_t totalFaultCount;
    SafeStateReason safeStateReason;

    /* E2E monitoring */
    boolean e2eTimeoutActive;
    Rte_TimestampType e2eTimeoutStart;

    /* FTTI tracking */
    Rte_TimestampType firstFaultTime;
    boolean fttiActive;

    /* Ambient light for safe state decision */
    uint16_t lastAmbientLight;
    boolean isDaytime;

    /* Timing */
    Rte_TimestampType safeStateEntryTime;
    Rte_TimestampType currentTime;

    /* Global status */
    SafetyStatusType globalStatus;
    WdgM_GlobalStatusType wdgmStatus;
} Rte_SafetyMonitor_DataType;

/*============================================================================*
 * RTE API - SENDER/RECEIVER PORTS
 *============================================================================*/

/**
 * @brief Read FLM state
 * @param[out] state Pointer to receive FLM state
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_SafetyMonitor_FLMState(FLM_StateType* state);

/**
 * @brief Read E2E status from SwitchEvent
 * @param[out] status Pointer to receive E2E status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_SafetyMonitor_E2EStatus(E2E_P01CheckStatusType* status);

/**
 * @brief Read headlight fault status
 * @param[out] status Pointer to receive fault status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_SafetyMonitor_HeadlightFault(HeadlightFaultStatus* status);

/**
 * @brief Read ambient light signal status
 * @param[out] status Pointer to receive signal status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_SafetyMonitor_AmbientStatus(SignalStatus* status);

/**
 * @brief Read current ambient light value
 * @param[out] level Pointer to receive ambient light level
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_SafetyMonitor_AmbientLight(AmbientLightLevel* level);

/**
 * @brief Write global safety status
 * @param[in] status Safety status to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_SafetyMonitor_GlobalStatus(SafetyStatusType status);

/**
 * @brief Write safe state active flag
 * @param[in] active Safe state active flag
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_SafetyMonitor_SafeStateActive(boolean active);

/**
 * @brief Write safe state headlight command
 * @param[in] command Headlight command for safe state
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_SafetyMonitor_SafeStateCommand(HeadlightCommand command);

/*============================================================================*
 * RTE API - CLIENT/SERVER PORTS
 *============================================================================*/

/**
 * @brief Get WdgM global status
 * @param[out] status Pointer to receive WdgM status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SafetyMonitor_WdgM_GetGlobalStatus(
    WdgM_GlobalStatusType* status
);

/**
 * @brief Get WdgM local status for supervised entity
 * @param[in] SEId Supervised Entity ID
 * @param[out] status Pointer to receive local status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SafetyMonitor_WdgM_GetLocalStatus(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_LocalStatusType* status
);

/**
 * @brief Report checkpoint to Watchdog Manager
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SafetyMonitor_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Report event to Diagnostic Event Manager
 * @param[in] eventId Event identifier
 * @param[in] eventStatus Event status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SafetyMonitor_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
);

/**
 * @brief Trigger ECU reset through BswM
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SafetyMonitor_BswM_RequestReset(void);

/*============================================================================*
 * RTE API - INTERNAL (IRV)
 *============================================================================*/

/**
 * @brief Get current system time
 * @return Current timestamp in milliseconds
 */
Rte_TimestampType Rte_IrvRead_SafetyMonitor_SystemTime(void);

/*============================================================================*
 * RUNNABLE DECLARATIONS
 *============================================================================*/

/**
 * @brief Initialization runnable
 * @details Called once during ECU startup
 */
void SafetyMonitor_Init(void);

/**
 * @brief Main function runnable
 * @details Called periodically every 5ms
 *          [ECU17] Monitors FTTI of 200ms
 */
void SafetyMonitor_MainFunction(void);

/**
 * @brief Get global safety status
 * @return Current global safety status
 */
SafetyStatusType SafetyMonitor_GetGlobalStatus(void);

/**
 * @brief Trigger safe state transition
 * @param[in] reason Reason for safe state entry
 */
void SafetyMonitor_TriggerSafeState(SafeStateReason reason);

/**
 * @brief Check if in safe state
 * @return TRUE if in safe state
 */
boolean SafetyMonitor_IsInSafeState(void);

/**
 * @brief Get safe state reason
 * @return Reason for current safe state
 */
SafeStateReason SafetyMonitor_GetSafeStateReason(void);

/*============================================================================*
 * SUPERVISION ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Supervision Entity ID for SafetyMonitor */
#define SAFETYMONITOR_SE_ID             0x0005U

/** @brief Checkpoint ID for main function entry */
#define SAFETYMONITOR_CP_MAIN_ENTRY     0x0001U

/** @brief Checkpoint ID for status aggregation */
#define SAFETYMONITOR_CP_AGGREGATION    0x0002U

/** @brief Checkpoint ID for main function exit */
#define SAFETYMONITOR_CP_MAIN_EXIT      0x0003U

/*============================================================================*
 * DEM EVENT CONFIGURATION
 *============================================================================*/

/** @brief WdgM supervision failed event ID */
#define SAFETYMONITOR_DEM_WDGM_FAILED   0x0008U

/** @brief Safe state entered event ID */
#define SAFETYMONITOR_DEM_SAFE_STATE    0x0009U

#endif /* RTE_SAFETYMONITOR_H */
