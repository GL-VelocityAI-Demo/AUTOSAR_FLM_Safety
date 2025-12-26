/**
 * @file Adc.cpp
 * @brief AUTOSAR ADC Driver Implementation
 * @details MCAL ADC driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Adc.h"
#include <cstring>

/*============================================================================*
 * LOCAL TYPES AND MACROS
 *============================================================================*/

/** @brief Default simulated ambient light value (mid-range) */
#define ADC_SIM_DEFAULT_AMBIENT             2000U

/** @brief Default simulated current sense value */
#define ADC_SIM_DEFAULT_CURRENT             500U

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean Adc_Initialized = FALSE;

/** @brief Configuration pointer */
static const Adc_ConfigType* Adc_ConfigPtr = NULL_PTR;

/** @brief Simulated ADC values */
static Adc_ValueGroupType Adc_SimValues[ADC_NUM_CHANNELS];

/** @brief Group status */
static Adc_StatusType Adc_GroupStatus[ADC_NUM_CHANNELS];

/** @brief Group conversion results */
static Adc_ValueGroupType Adc_GroupResults[ADC_NUM_CHANNELS];

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize ADC driver
 */
void Adc_Init(const Adc_ConfigType* ConfigPtr) {
    uint8_t i;

    if (ConfigPtr == NULL_PTR) {
        /* Development error - null pointer */
        return;
    }

    /* Store configuration */
    Adc_ConfigPtr = ConfigPtr;

    /* Initialize simulated values */
    for (i = 0U; i < ADC_NUM_CHANNELS; i++) {
        Adc_SimValues[i] = 0U;
        Adc_GroupStatus[i] = ADC_IDLE;
        Adc_GroupResults[i] = 0U;
    }

    /* Set default simulation values */
    Adc_SimValues[0] = ADC_SIM_DEFAULT_AMBIENT;  /* Ambient light channel */
    Adc_SimValues[1] = ADC_SIM_DEFAULT_CURRENT;  /* Current sense channel */

    Adc_Initialized = TRUE;
}

/**
 * @brief De-initialize ADC driver
 */
void Adc_DeInit(void) {
    uint8_t i;

    if (!Adc_Initialized) {
        return;
    }

    /* Reset all states */
    for (i = 0U; i < ADC_NUM_CHANNELS; i++) {
        Adc_SimValues[i] = 0U;
        Adc_GroupStatus[i] = ADC_IDLE;
        Adc_GroupResults[i] = 0U;
    }

    Adc_ConfigPtr = NULL_PTR;
    Adc_Initialized = FALSE;
}

/**
 * @brief Start group conversion
 */
void Adc_StartGroupConversion(Adc_GroupType Group) {
    if (!Adc_Initialized) {
        return;
    }

    if (Group >= ADC_NUM_CHANNELS) {
        return;
    }

    /* In simulation, conversion completes immediately */
    Adc_GroupStatus[Group] = ADC_BUSY;

    /* Copy simulated value to result */
    Adc_GroupResults[Group] = Adc_SimValues[Group];

    /* Mark as completed (simulated instant conversion) */
    Adc_GroupStatus[Group] = ADC_COMPLETED;
}

/**
 * @brief Stop group conversion
 */
void Adc_StopGroupConversion(Adc_GroupType Group) {
    if (!Adc_Initialized) {
        return;
    }

    if (Group >= ADC_NUM_CHANNELS) {
        return;
    }

    Adc_GroupStatus[Group] = ADC_IDLE;
}

/**
 * @brief Read group conversion results
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    if (!Adc_Initialized) {
        return E_NOT_OK;
    }

    if (DataBufferPtr == NULL_PTR) {
        return E_NOT_OK;
    }

    if (Group >= ADC_NUM_CHANNELS) {
        return E_NOT_OK;
    }

    if (Adc_GroupStatus[Group] != ADC_COMPLETED) {
        return E_NOT_OK;
    }

    /* Return the result */
    *DataBufferPtr = Adc_GroupResults[Group];

    /* Reset status to idle after read */
    Adc_GroupStatus[Group] = ADC_IDLE;

    return E_OK;
}

/**
 * @brief Get group status
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group) {
    if (!Adc_Initialized) {
        return ADC_IDLE;
    }

    if (Group >= ADC_NUM_CHANNELS) {
        return ADC_IDLE;
    }

    return Adc_GroupStatus[Group];
}

/**
 * @brief Enable hardware trigger
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group) {
    /* Not implemented in simulation */
    STD_UNUSED(Group);
}

/**
 * @brief Disable hardware trigger
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group) {
    /* Not implemented in simulation */
    STD_UNUSED(Group);
}

/**
 * @brief Get version information
 */
void Adc_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    if (versioninfo == NULL_PTR) {
        return;
    }

    versioninfo->vendorID = 0U;
    versioninfo->moduleID = 123U;  /* ADC module ID */
    versioninfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = ADC_SW_PATCH_VERSION;
}

/*============================================================================*
 * SIMULATION FUNCTIONS
 *============================================================================*/

/**
 * @brief Set simulated ADC value
 */
void Adc_SimSetValue(Adc_ChannelType Channel, Adc_ValueGroupType Value) {
    if (Channel >= ADC_NUM_CHANNELS) {
        return;
    }

    /* Clamp to valid range */
    if (Value > ADC_MAX_VALUE) {
        Value = ADC_MAX_VALUE;
    }

    Adc_SimValues[Channel] = Value;
}

/**
 * @brief Get current simulated ADC value
 */
Adc_ValueGroupType Adc_SimGetValue(Adc_ChannelType Channel) {
    if (Channel >= ADC_NUM_CHANNELS) {
        return 0U;
    }

    return Adc_SimValues[Channel];
}

/**
 * @brief Trigger conversion complete
 */
void Adc_SimTriggerComplete(Adc_GroupType Group) {
    if (Group >= ADC_NUM_CHANNELS) {
        return;
    }

    /* Copy simulated value and mark complete */
    Adc_GroupResults[Group] = Adc_SimValues[Group];
    Adc_GroupStatus[Group] = ADC_COMPLETED;
}
