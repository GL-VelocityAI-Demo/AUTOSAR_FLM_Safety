/**
 * @file Rte_SwitchEvent.h
 * @brief RTE Interface for SwitchEvent SWC
 * @details Provides RTE API for the SwitchEvent software component (ASIL B)
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - This component is safety-relevant
 */

#ifndef RTE_SWITCHEVENT_H
#define RTE_SWITCHEVENT_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte_Type.h"

/*============================================================================*
 * COMPONENT DATA STRUCTURES
 *============================================================================*/

/**
 * @brief SwitchEvent Component Data Structure
 */
typedef struct {
    /* Internal state */
    boolean isInitialized;

    /* Input data */
    Rte_CanMessageType canMessage;

    /* Output data */
    LightSwitchStatus lightSwitchStatus;
    E2E_P01CheckStatusType e2eCheckStatus;

    /* Timing */
    Rte_TimestampType lastMessageTime;
    Rte_TimestampType currentTime;

    /* Error counters */
    uint8_t e2eErrorCount;
    uint8_t timeoutCount;
} Rte_SwitchEvent_DataType;

/*============================================================================*
 * RTE API - SENDER/RECEIVER PORTS
 *============================================================================*/

/**
 * @brief Read CAN message from COM layer
 * @param[out] message Pointer to receive CAN message data
 * @return RTE_E_OK on success, error code otherwise
 */
Rte_StatusType Rte_Read_SwitchEvent_CanLightSwitchMsg(Rte_CanMessageType* message);

/**
 * @brief Write validated light switch status
 * @param[in] status Light switch status to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_SwitchEvent_LightSwitchStatus(const LightSwitchStatus* status);

/**
 * @brief Write E2E check status
 * @param[in] e2eStatus E2E protection status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_SwitchEvent_E2EStatus(E2E_P01CheckStatusType e2eStatus);

/*============================================================================*
 * RTE API - CLIENT/SERVER PORTS
 *============================================================================*/

/**
 * @brief Report checkpoint to Watchdog Manager
 * @details [SysSafReq03] Alive supervision checkpoint
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SwitchEvent_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Report event to Diagnostic Event Manager
 * @param[in] eventId Event identifier
 * @param[in] eventStatus Event status (passed/failed)
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_SwitchEvent_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
);

/*============================================================================*
 * RTE API - MODE PORTS
 *============================================================================*/

/**
 * @brief Get current communication mode
 * @param[out] mode Current mode
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Mode_SwitchEvent_ComMMode(ComM_ModeType* mode);

/*============================================================================*
 * RTE API - INTERNAL (IRV)
 *============================================================================*/

/**
 * @brief Get current system time
 * @return Current timestamp in milliseconds
 */
Rte_TimestampType Rte_IrvRead_SwitchEvent_SystemTime(void);

/*============================================================================*
 * RUNNABLE DECLARATIONS
 *============================================================================*/

/**
 * @brief Initialization runnable
 * @details Called once during ECU startup
 */
void SwitchEvent_Init(void);

/**
 * @brief Main function runnable
 * @details Called periodically every 10ms
 *          [SysSafReq01] CAN timeout detection 50ms
 */
void SwitchEvent_MainFunction(void);

/*============================================================================*
 * SUPERVISION ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Supervision Entity ID for SwitchEvent */
#define SWITCHEVENT_SE_ID           0x0001U

/** @brief Checkpoint ID for main function entry */
#define SWITCHEVENT_CP_MAIN_ENTRY   0x0001U

/** @brief Checkpoint ID for main function exit */
#define SWITCHEVENT_CP_MAIN_EXIT    0x0002U

/*============================================================================*
 * DEM EVENT CONFIGURATION
 *============================================================================*/

/** @brief E2E failure event ID */
#define SWITCHEVENT_DEM_E2E_FAILED      0x0001U

/** @brief CAN timeout event ID */
#define SWITCHEVENT_DEM_CAN_TIMEOUT     0x0007U

#endif /* RTE_SWITCHEVENT_H */
