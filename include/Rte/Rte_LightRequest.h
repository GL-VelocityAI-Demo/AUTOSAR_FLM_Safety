/**
 * @file Rte_LightRequest.h
 * @brief RTE Interface for LightRequest SWC
 * @details Provides RTE API for the LightRequest software component (ASIL A)
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL A - This component is safety-relevant
 */

#ifndef RTE_LIGHTREQUEST_H
#define RTE_LIGHTREQUEST_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte_Type.h"

/*============================================================================*
 * CONFIGURATION CONSTANTS
 *============================================================================*/

/** @brief Number of ADC samples for averaging [FunSafReq01-02] */
#define LIGHTREQUEST_ADC_SAMPLES        4U

/** @brief ADC buffer size */
#define LIGHTREQUEST_ADC_BUFFER_SIZE    8U

/*============================================================================*
 * COMPONENT DATA STRUCTURES
 *============================================================================*/

/**
 * @brief LightRequest Component Data Structure
 */
typedef struct {
    /* Internal state */
    boolean isInitialized;

    /* ADC data */
    uint16_t adcBuffer[LIGHTREQUEST_ADC_BUFFER_SIZE];
    uint8_t adcBufferIndex;
    uint16_t adcFilteredValue;
    uint16_t adcPreviousValue;

    /* Output data */
    AmbientLightLevel ambientLight;
    SignalStatus signalStatus;

    /* Timing */
    Rte_TimestampType lastReadTime;
    Rte_TimestampType currentTime;

    /* Plausibility data */
    uint16_t rateOfChange;
    uint8_t plausibilityErrorCount;
} Rte_LightRequest_DataType;

/*============================================================================*
 * RTE API - SENDER/RECEIVER PORTS
 *============================================================================*/

/**
 * @brief Read raw ADC value from MCAL ADC driver
 * @param[out] adcValue Pointer to receive ADC value (0-4095)
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Read_LightRequest_AdcAmbientLight(uint16_t* adcValue);

/**
 * @brief Write validated ambient light level
 * @param[in] level Ambient light level to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_LightRequest_AmbientLightLevel(const AmbientLightLevel* level);

/**
 * @brief Write signal status
 * @param[in] status Signal status to write
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Write_LightRequest_SignalStatus(SignalStatus status);

/*============================================================================*
 * RTE API - CLIENT/SERVER PORTS
 *============================================================================*/

/**
 * @brief Request ADC conversion
 * @param[in] channel ADC channel number
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_LightRequest_Adc_StartConversion(uint8_t channel);

/**
 * @brief Get ADC conversion result
 * @param[in] channel ADC channel number
 * @param[out] value Conversion result
 * @return RTE_E_OK on success, RTE_E_NO_DATA if not ready
 */
Rte_StatusType Rte_Call_LightRequest_Adc_GetResult(uint8_t channel, uint16_t* value);

/**
 * @brief Report checkpoint to Watchdog Manager
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_LightRequest_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Report event to Diagnostic Event Manager
 * @param[in] eventId Event identifier
 * @param[in] eventStatus Event status
 * @return RTE_E_OK on success
 */
Rte_StatusType Rte_Call_LightRequest_Dem_SetEventStatus(
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
Rte_TimestampType Rte_IrvRead_LightRequest_SystemTime(void);

/*============================================================================*
 * RUNNABLE DECLARATIONS
 *============================================================================*/

/**
 * @brief Initialization runnable
 * @details Called once during ECU startup
 */
void LightRequest_Init(void);

/**
 * @brief Main function runnable
 * @details Called periodically every 20ms
 *          [FunSafReq01-02] Ambient light sensor validation
 */
void LightRequest_MainFunction(void);

/**
 * @brief Get current ambient light level
 * @return Current ambient light level with validity
 */
AmbientLightLevel LightRequest_GetAmbientLight(void);

/**
 * @brief Get current signal status
 * @return Current signal status
 */
SignalStatus LightRequest_GetSignalStatus(void);

/*============================================================================*
 * SUPERVISION ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Supervision Entity ID for LightRequest */
#define LIGHTREQUEST_SE_ID              0x0002U

/** @brief Checkpoint ID for main function entry */
#define LIGHTREQUEST_CP_MAIN_ENTRY      0x0001U

/** @brief Checkpoint ID for main function exit */
#define LIGHTREQUEST_CP_MAIN_EXIT       0x0002U

/*============================================================================*
 * DEM EVENT CONFIGURATION
 *============================================================================*/

/** @brief Open circuit event ID */
#define LIGHTREQUEST_DEM_OPEN_CIRCUIT       0x0002U

/** @brief Short circuit event ID */
#define LIGHTREQUEST_DEM_SHORT_CIRCUIT      0x0003U

/** @brief Plausibility error event ID */
#define LIGHTREQUEST_DEM_PLAUSIBILITY       0x0004U

/*============================================================================*
 * ADC CHANNEL CONFIGURATION
 *============================================================================*/

/** @brief ADC channel for ambient light sensor */
#define LIGHTREQUEST_ADC_CHANNEL        0x00U

#endif /* RTE_LIGHTREQUEST_H */
