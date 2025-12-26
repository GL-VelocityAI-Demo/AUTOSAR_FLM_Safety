/**
 * @file LightRequest.h
 * @brief LightRequest SWC Interface
 * @details Reads ambient light sensor and performs plausibility checks
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL A - [FunSafReq01-02] Ambient light sensor validation
 */

#ifndef LIGHTREQUEST_H
#define LIGHTREQUEST_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte/Rte_LightRequest.h"
#include "FLM_Config.h"

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Rate limit check period in cycles (100ms at 20ms period) */
#define LIGHTREQUEST_RATE_CHECK_CYCLES      5U

/** @brief Plausibility error debounce count */
#define LIGHTREQUEST_PLAUSIBILITY_DEBOUNCE  3U

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief LightRequest internal state
 */
typedef struct {
    /* Initialization */
    boolean isInitialized;

    /* ADC data and filtering */
    uint16_t adcBuffer[LIGHTREQUEST_ADC_BUFFER_SIZE];
    uint8_t adcBufferIndex;
    uint8_t adcSampleCount;
    uint16_t adcFilteredValue;
    uint16_t adcRawValue;

    /* Rate of change tracking */
    uint16_t previousFilteredValue;
    uint16_t rateOfChange;
    uint8_t rateCheckCounter;

    /* Output data */
    AmbientLightLevel ambientLight;
    SignalStatus signalStatus;

    /* Plausibility */
    uint8_t plausibilityErrorCount;
    boolean plausibilityFault;

    /* Timing */
    uint32_t currentTimestamp;

    /* Conversion pending flag */
    boolean conversionPending;
} LightRequest_StateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize LightRequest component
 * @details Called once at ECU startup
 */
void LightRequest_Init(void);

/**
 * @brief Main function for LightRequest
 * @details Called every 20ms
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

/**
 * @brief Get filtered ADC value
 * @return Filtered ADC value
 */
uint16_t LightRequest_GetFilteredAdcValue(void);

/**
 * @brief Get rate of change
 * @return Current rate of change (ADC counts per 100ms)
 */
uint16_t LightRequest_GetRateOfChange(void);

/**
 * @brief Check if plausibility fault is active
 * @return TRUE if plausibility fault is active
 */
boolean LightRequest_IsPlausibilityFault(void);

/**
 * @brief Set simulated ADC value (for testing)
 * @param[in] value ADC value to simulate
 */
void LightRequest_SimSetAdcValue(uint16_t value);

/**
 * @brief Get component state (for testing)
 * @return Pointer to internal state
 */
const LightRequest_StateType* LightRequest_GetState(void);

#endif /* LIGHTREQUEST_H */
