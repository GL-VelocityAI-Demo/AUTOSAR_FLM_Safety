/**
 * @file LightRequest.cpp
 * @brief LightRequest SWC Implementation
 * @details Reads ambient light sensor and performs plausibility checks
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL A - [FunSafReq01-02] Ambient light sensor validation
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "LightRequest.h"
#include "MCAL/Adc/Adc.h"
#include "Dem_Cfg.h"
#include <cstring>

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Component internal state */
static LightRequest_StateType LightRequest_State;

/** @brief System time counter */
static uint32_t LightRequest_SystemTime = 0U;

/** @brief Simulated ADC value */
static uint16_t LightRequest_SimAdcValue = 2000U;
static boolean LightRequest_SimAdcEnabled = FALSE;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void LightRequest_ReadAdc(void);
static void LightRequest_ApplyFilter(void);
static void LightRequest_CheckOpenCircuit(void);
static void LightRequest_CheckShortCircuit(void);
static void LightRequest_CheckPlausibility(void);
static void LightRequest_UpdateOutput(void);
static void LightRequest_ReportDemEvents(void);
static uint16_t LightRequest_AdcToLux(uint16_t adcValue);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize LightRequest component
 */
void LightRequest_Init(void) {
    uint8_t i;

    /* Clear state structure */
    (void)memset(&LightRequest_State, 0, sizeof(LightRequest_State));

    /* Initialize ADC buffer */
    for (i = 0U; i < LIGHTREQUEST_ADC_BUFFER_SIZE; i++) {
        LightRequest_State.adcBuffer[i] = 0U;
    }
    LightRequest_State.adcBufferIndex = 0U;
    LightRequest_State.adcSampleCount = 0U;
    LightRequest_State.adcFilteredValue = 0U;
    LightRequest_State.adcRawValue = 0U;

    /* Initialize rate of change tracking */
    LightRequest_State.previousFilteredValue = 0U;
    LightRequest_State.rateOfChange = 0U;
    LightRequest_State.rateCheckCounter = 0U;

    /* Initialize output */
    LightRequest_State.ambientLight.adcValue = 0U;
    LightRequest_State.ambientLight.luxValue = 0U;
    LightRequest_State.ambientLight.isValid = FALSE;
    LightRequest_State.signalStatus = SIGNAL_STATUS_INVALID;

    /* Initialize plausibility */
    LightRequest_State.plausibilityErrorCount = 0U;
    LightRequest_State.plausibilityFault = FALSE;

    /* Mark as initialized */
    LightRequest_State.isInitialized = TRUE;
}

/**
 * @brief Main function for LightRequest
 * @details [FunSafReq01-02] Ambient light sensor validation
 */
void LightRequest_MainFunction(void) {
    if (!LightRequest_State.isInitialized) {
        return;
    }

    /* Update timestamp */
    LightRequest_State.currentTimestamp = LightRequest_SystemTime;
    LightRequest_SystemTime += FLM_AMBIENT_LIGHT_PERIOD_MS;

    /* Read ADC value */
    LightRequest_ReadAdc();

    /* Apply signal conditioning (averaging) */
    LightRequest_ApplyFilter();

    /* Check for open circuit [FunSafReq01-02] */
    LightRequest_CheckOpenCircuit();

    /* Check for short circuit [FunSafReq01-02] */
    LightRequest_CheckShortCircuit();

    /* Check plausibility (rate of change) [FunSafReq01-02] */
    LightRequest_CheckPlausibility();

    /* Update output values */
    LightRequest_UpdateOutput();

    /* Report DEM events */
    LightRequest_ReportDemEvents();
}

/**
 * @brief Read ADC value
 */
static void LightRequest_ReadAdc(void) {
    Adc_ValueGroupType adcValue;
    Std_ReturnType result;

    /* Check if using simulated value */
    if (LightRequest_SimAdcEnabled) {
        LightRequest_State.adcRawValue = LightRequest_SimAdcValue;
        return;
    }

    /* Start ADC conversion */
    Adc_StartGroupConversion(LIGHTREQUEST_ADC_CHANNEL);

    /* Read result (in simulation, conversion is instant) */
    result = Adc_ReadGroup(LIGHTREQUEST_ADC_CHANNEL, &adcValue);

    if (result == E_OK) {
        LightRequest_State.adcRawValue = adcValue;
    }
    /* If read fails, keep previous value */
}

/**
 * @brief Apply signal conditioning filter
 * @details Averaging over FLM_ADC_SAMPLES samples
 */
static void LightRequest_ApplyFilter(void) {
    uint32_t sum = 0U;
    uint8_t i;
    uint8_t count;

    /* Add new sample to buffer */
    LightRequest_State.adcBuffer[LightRequest_State.adcBufferIndex] =
        LightRequest_State.adcRawValue;

    /* Update buffer index (circular) */
    LightRequest_State.adcBufferIndex =
        (LightRequest_State.adcBufferIndex + 1U) % FLM_ADC_SAMPLES;

    /* Track sample count for startup */
    if (LightRequest_State.adcSampleCount < FLM_ADC_SAMPLES) {
        LightRequest_State.adcSampleCount++;
    }

    /* Calculate average */
    count = LightRequest_State.adcSampleCount;
    for (i = 0U; i < count; i++) {
        sum += LightRequest_State.adcBuffer[i];
    }

    LightRequest_State.adcFilteredValue =
        static_cast<uint16_t>(sum / count);
}

/**
 * @brief Check for open circuit condition
 * @details [FunSafReq01-02] ADC < 100 indicates open circuit
 */
static void LightRequest_CheckOpenCircuit(void) {
    if (LightRequest_State.adcFilteredValue < FLM_AMBIENT_OPEN_CIRCUIT) {
        LightRequest_State.signalStatus = SIGNAL_STATUS_OPEN_CIRCUIT;
        LightRequest_State.ambientLight.isValid = FALSE;
    }
}

/**
 * @brief Check for short circuit condition
 * @details [FunSafReq01-02] ADC > 3995 indicates short circuit
 */
static void LightRequest_CheckShortCircuit(void) {
    if (LightRequest_State.adcFilteredValue > FLM_AMBIENT_SHORT_CIRCUIT) {
        LightRequest_State.signalStatus = SIGNAL_STATUS_SHORT_CIRCUIT;
        LightRequest_State.ambientLight.isValid = FALSE;
    }
}

/**
 * @brief Check rate of change plausibility
 * @details [FunSafReq01-02] Rate limit: max 500 LSB per 100ms
 */
static void LightRequest_CheckPlausibility(void) {
    int32_t delta;

    /* Increment rate check counter */
    LightRequest_State.rateCheckCounter++;

    /* Check rate every 100ms (5 cycles at 20ms) */
    if (LightRequest_State.rateCheckCounter >= LIGHTREQUEST_RATE_CHECK_CYCLES) {
        LightRequest_State.rateCheckCounter = 0U;

        /* Calculate delta from previous value */
        delta = static_cast<int32_t>(LightRequest_State.adcFilteredValue) -
                static_cast<int32_t>(LightRequest_State.previousFilteredValue);

        /* Get absolute value */
        if (delta < 0) {
            delta = -delta;
        }

        LightRequest_State.rateOfChange = static_cast<uint16_t>(delta);

        /* Check against limit */
        if (LightRequest_State.rateOfChange > FLM_AMBIENT_RATE_LIMIT) {
            /* Plausibility error - increment debounce counter */
            if (LightRequest_State.plausibilityErrorCount <
                LIGHTREQUEST_PLAUSIBILITY_DEBOUNCE) {
                LightRequest_State.plausibilityErrorCount++;
            }

            if (LightRequest_State.plausibilityErrorCount >=
                LIGHTREQUEST_PLAUSIBILITY_DEBOUNCE) {
                LightRequest_State.plausibilityFault = TRUE;
                LightRequest_State.signalStatus = SIGNAL_STATUS_PLAUSIBILITY;
                LightRequest_State.ambientLight.isValid = FALSE;
            }
        } else {
            /* Reset debounce counter */
            LightRequest_State.plausibilityErrorCount = 0U;
            LightRequest_State.plausibilityFault = FALSE;
        }

        /* Store current value for next comparison */
        LightRequest_State.previousFilteredValue =
            LightRequest_State.adcFilteredValue;
    }
}

/**
 * @brief Update output values
 */
static void LightRequest_UpdateOutput(void) {
    /* If no faults detected, signal is valid */
    if ((LightRequest_State.signalStatus != SIGNAL_STATUS_OPEN_CIRCUIT) &&
        (LightRequest_State.signalStatus != SIGNAL_STATUS_SHORT_CIRCUIT) &&
        (LightRequest_State.signalStatus != SIGNAL_STATUS_PLAUSIBILITY)) {

        /* Need minimum samples before valid */
        if (LightRequest_State.adcSampleCount >= FLM_ADC_SAMPLES) {
            LightRequest_State.signalStatus = SIGNAL_STATUS_VALID;
            LightRequest_State.ambientLight.isValid = TRUE;
        } else {
            LightRequest_State.signalStatus = SIGNAL_STATUS_INVALID;
            LightRequest_State.ambientLight.isValid = FALSE;
        }
    }

    /* Update ADC value in output */
    LightRequest_State.ambientLight.adcValue =
        LightRequest_State.adcFilteredValue;

    /* Convert to lux */
    LightRequest_State.ambientLight.luxValue =
        LightRequest_AdcToLux(LightRequest_State.adcFilteredValue);
}

/**
 * @brief Convert ADC value to lux
 * @param[in] adcValue ADC value (0-4095)
 * @return Lux value
 */
static uint16_t LightRequest_AdcToLux(uint16_t adcValue) {
    /* Simple linear conversion for simulation */
    /* ADC 800 ~ 200 lux (threshold ON) */
    /* ADC 1000 ~ 250 lux (threshold OFF) */
    /* Approximate: lux = ADC / 4 */
    return adcValue / 4U;
}

/**
 * @brief Report events to DEM
 */
static void LightRequest_ReportDemEvents(void) {
    /* Report open circuit */
    if (LightRequest_State.signalStatus == SIGNAL_STATUS_OPEN_CIRCUIT) {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_OPEN_CIRCUIT,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_OPEN_CIRCUIT,
            DEM_EVENT_STATUS_PASSED
        );
    }

    /* Report short circuit */
    if (LightRequest_State.signalStatus == SIGNAL_STATUS_SHORT_CIRCUIT) {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_SHORT_CIRCUIT,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_SHORT_CIRCUIT,
            DEM_EVENT_STATUS_PASSED
        );
    }

    /* Report plausibility error */
    if (LightRequest_State.plausibilityFault) {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_PLAUSIBILITY,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_LightRequest_Dem_SetEventStatus(
            DEM_EVENT_AMBIENTLIGHT_PLAUSIBILITY,
            DEM_EVENT_STATUS_PASSED
        );
    }
}

/*============================================================================*
 * PUBLIC INTERFACE FUNCTIONS
 *============================================================================*/

AmbientLightLevel LightRequest_GetAmbientLight(void) {
    return LightRequest_State.ambientLight;
}

SignalStatus LightRequest_GetSignalStatus(void) {
    return LightRequest_State.signalStatus;
}

uint16_t LightRequest_GetFilteredAdcValue(void) {
    return LightRequest_State.adcFilteredValue;
}

uint16_t LightRequest_GetRateOfChange(void) {
    return LightRequest_State.rateOfChange;
}

boolean LightRequest_IsPlausibilityFault(void) {
    return LightRequest_State.plausibilityFault;
}

void LightRequest_SimSetAdcValue(uint16_t value) {
    LightRequest_SimAdcValue = value;
    LightRequest_SimAdcEnabled = TRUE;
}

const LightRequest_StateType* LightRequest_GetState(void) {
    return &LightRequest_State;
}

/*============================================================================*
 * RTE PORT IMPLEMENTATIONS (STUBS)
 *============================================================================*/

Rte_StatusType Rte_Read_LightRequest_AdcAmbientLight(uint16_t* adcValue) {
    if (adcValue == NULL_PTR) {
        return RTE_E_INVALID;
    }
    *adcValue = LightRequest_State.adcFilteredValue;
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_LightRequest_AmbientLightLevel(const AmbientLightLevel* level) {
    STD_UNUSED(level);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_LightRequest_SignalStatus(SignalStatus status) {
    STD_UNUSED(status);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_LightRequest_Adc_StartConversion(uint8_t channel) {
    Adc_StartGroupConversion(channel);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_LightRequest_Adc_GetResult(uint8_t channel, uint16_t* value) {
    Adc_ValueGroupType adcValue;
    if (Adc_ReadGroup(channel, &adcValue) == E_OK) {
        *value = adcValue;
        return RTE_E_OK;
    }
    return RTE_E_NO_DATA;
}

Rte_StatusType Rte_Call_LightRequest_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    STD_UNUSED(SEId);
    STD_UNUSED(CPId);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_LightRequest_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
) {
    STD_UNUSED(eventId);
    STD_UNUSED(eventStatus);
    return RTE_E_OK;
}

Rte_TimestampType Rte_IrvRead_LightRequest_SystemTime(void) {
    return LightRequest_SystemTime;
}
