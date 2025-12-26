/**
 * @file Rte_Headlight.h
 * @brief RTE Interface for Headlight SWC
 * @details Provides RTE API for the Headlight software component (ASIL B)
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - This component is safety-relevant
 */

#ifndef RTE_HEADLIGHT_H
#define RTE_HEADLIGHT_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte_Type.h"

/*============================================================================*
 * COMPONENT DATA STRUCTURES
 *============================================================================*/

/**
 * @brief Headlight Component Data Structure
 */
typedef struct {
    /* Internal state */
    boolean isInitialized;

    /* Command data */
    HeadlightCommand currentCommand;
    HeadlightCommand requestedCommand;

    /* Feedback data */
    uint16_t feedbackCurrent;       /**< Current sense in mA */
    boolean outputState;            /**< Actual output state */

    /* Fault detection */
    HeadlightFaultStatus faultStatus;
    Rte_TimestampType faultDetectStart;
    boolean faultConfirmed;
    uint8_t consecutiveFaultCycles;

    /* Timing */
    Rte_TimestampType commandTime;
    Rte_TimestampType currentTime;
} Rte_Headlight_DataType;

/*============================================================================*
 * CONFIGURATION CONSTANTS
 *============================================================================*/

/** @brief Fault detection time threshold in ms [SysSafReq10] */
#define HEADLIGHT_FAULT_DETECT_TIME_MS      20U

/** @brief Minimum current for ON detection (mA) */
#define HEADLIGHT_MIN_CURRENT_MA            100U

/** @brief Maximum current threshold (mA) - overcurrent */
#define HEADLIGHT_MAX_CURRENT_MA            15000U

/** @brief Open load current threshold (mA) */
#define HEADLIGHT_OPEN_LOAD_CURRENT_MA      50U

/** @brief Number of consecutive fault cycles for confirmation */
#define HEADLIGHT_FAULT_CONFIRM_CYCLES      2U

/*============================================================================*
 * RTE API - SENDER/RECEIVER PORTS
 *============================================================================*/

/**
 * @brief Read headlight command from FLM SWC
 * @param[out] command Pointer to receive headlight command
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_Headlight_Command(HeadlightCommand* command);

/**
 * @brief Write headlight fault status
 * @param[in] status Fault status to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_Headlight_FaultStatus(HeadlightFaultStatus status);

/**
 * @brief Write actual headlight state
 * @param[in] state Actual output state
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_Headlight_ActualState(boolean state);

/*============================================================================*
 * RTE API - CLIENT/SERVER PORTS
 *============================================================================*/

/**
 * @brief Set DIO output for headlight relay
 * @param[in] channelId DIO channel ID
 * @param[in] level Output level (STD_HIGH/STD_LOW)
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_Headlight_Dio_WriteChannel(
    uint8_t channelId,
    uint8_t level
);

/**
 * @brief Read DIO input for feedback
 * @param[in] channelId DIO channel ID
 * @param[out] level Current level
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_Headlight_Dio_ReadChannel(
    uint8_t channelId,
    uint8_t* level
);

/**
 * @brief Read current sense ADC value
 * @param[in] channel ADC channel
 * @param[out] value ADC value
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_Headlight_Adc_GetCurrentSense(
    uint8_t channel,
    uint16_t* value
);

/**
 * @brief Report checkpoint to Watchdog Manager
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_Headlight_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Report event to Diagnostic Event Manager
 * @param[in] eventId Event identifier
 * @param[in] eventStatus Event status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_Headlight_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
);

/*============================================================================*
 * RTE API - INTERNAL (IRV)
 *============================================================================*/

/**
 * @brief Get current system time
 * @return Current timestamp in milliseconds
 */
Rte_TimestampType Rte_IrvRead_Headlight_SystemTime(void);

/*============================================================================*
 * RUNNABLE DECLARATIONS
 *============================================================================*/

/**
 * @brief Initialization runnable
 * @details Called once during ECU startup
 */
void Headlight_Init(void);

/**
 * @brief Main function runnable
 * @details Called periodically every 10ms
 *          [SysSafReq10] Output stage diagnosis within 20ms
 */
void Headlight_MainFunction(void);

/**
 * @brief Set headlight command
 * @param[in] cmd Headlight command
 */
void Headlight_SetCommand(HeadlightCommand cmd);

/**
 * @brief Get current fault status
 * @return Current fault status
 */
HeadlightFaultStatus Headlight_GetFaultStatus(void);

/**
 * @brief Get actual output state
 * @return TRUE if headlight is ON
 */
boolean Headlight_GetActualState(void);

/*============================================================================*
 * SUPERVISION ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Supervision Entity ID for Headlight */
#define HEADLIGHT_SE_ID                 0x0004U

/** @brief Checkpoint ID for main function entry */
#define HEADLIGHT_CP_MAIN_ENTRY         0x0001U

/** @brief Checkpoint ID for main function exit */
#define HEADLIGHT_CP_MAIN_EXIT          0x0002U

/*============================================================================*
 * DEM EVENT CONFIGURATION
 *============================================================================*/

/** @brief Open load event ID */
#define HEADLIGHT_DEM_OPEN_LOAD         0x0005U

/** @brief Short circuit event ID */
#define HEADLIGHT_DEM_SHORT_CIRCUIT     0x0006U

/*============================================================================*
 * DIO CHANNEL CONFIGURATION
 *============================================================================*/

/** @brief DIO channel for low beam relay */
#define HEADLIGHT_DIO_LOW_BEAM          0x00U

/** @brief DIO channel for high beam relay */
#define HEADLIGHT_DIO_HIGH_BEAM         0x01U

/** @brief ADC channel for current sense */
#define HEADLIGHT_ADC_CURRENT_SENSE     0x01U

#endif /* RTE_HEADLIGHT_H */
