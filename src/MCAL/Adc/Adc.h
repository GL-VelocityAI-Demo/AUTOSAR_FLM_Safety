/**
 * @file Adc.h
 * @brief AUTOSAR ADC Driver Interface
 * @details MCAL ADC driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef ADC_H
#define ADC_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"

/*============================================================================*
 * VERSION INFORMATION
 *============================================================================*/
#define ADC_AR_RELEASE_MAJOR_VERSION        23
#define ADC_AR_RELEASE_MINOR_VERSION        11

#define ADC_SW_MAJOR_VERSION                1
#define ADC_SW_MINOR_VERSION                0
#define ADC_SW_PATCH_VERSION                0

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Number of ADC channels */
#define ADC_NUM_CHANNELS                    8U

/** @brief ADC resolution in bits */
#define ADC_RESOLUTION_BITS                 12U

/** @brief Maximum ADC value */
#define ADC_MAX_VALUE                       ((1U << ADC_RESOLUTION_BITS) - 1U)

/** @brief Enable development error detection */
#define ADC_DEV_ERROR_DETECT                STD_ON

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief ADC channel type
 */
typedef uint8_t Adc_ChannelType;

/**
 * @brief ADC group type
 */
typedef uint8_t Adc_GroupType;

/**
 * @brief ADC value type
 */
typedef uint16_t Adc_ValueGroupType;

/**
 * @brief ADC conversion status type
 */
typedef enum {
    ADC_IDLE        = 0x00U,    /**< Group is idle */
    ADC_BUSY        = 0x01U,    /**< Conversion ongoing */
    ADC_COMPLETED   = 0x02U,    /**< Conversion completed */
    ADC_STREAM_COMPLETED = 0x03U /**< Stream completed */
} Adc_StatusType;

/**
 * @brief ADC trigger source type
 */
typedef enum {
    ADC_TRIGG_SRC_SW = 0x00U,   /**< Software trigger */
    ADC_TRIGG_SRC_HW = 0x01U    /**< Hardware trigger */
} Adc_TriggerSourceType;

/**
 * @brief ADC conversion mode type
 */
typedef enum {
    ADC_CONV_MODE_ONESHOT     = 0x00U,  /**< One-shot conversion */
    ADC_CONV_MODE_CONTINUOUS  = 0x01U   /**< Continuous conversion */
} Adc_GroupConvModeType;

/**
 * @brief ADC priority type
 */
typedef uint8_t Adc_GroupPriorityType;

/**
 * @brief ADC channel configuration type
 */
typedef struct {
    Adc_ChannelType ChannelId;      /**< Channel identifier */
    uint16_t SamplingTime;          /**< Sampling time in clock cycles */
    boolean Enabled;                /**< Channel enabled */
} Adc_ChannelConfigType;

/**
 * @brief ADC group configuration type
 */
typedef struct {
    Adc_GroupType GroupId;              /**< Group identifier */
    Adc_TriggerSourceType TriggerSource;/**< Trigger source */
    Adc_GroupConvModeType ConvMode;     /**< Conversion mode */
    Adc_GroupPriorityType Priority;     /**< Group priority */
    uint8_t NumChannels;                /**< Number of channels in group */
    const Adc_ChannelType* Channels;    /**< Pointer to channel list */
} Adc_GroupConfigType;

/**
 * @brief ADC configuration type
 */
typedef struct {
    uint8_t NumGroups;                      /**< Number of groups */
    const Adc_GroupConfigType* Groups;      /**< Pointer to group config */
    uint8_t NumChannels;                    /**< Number of channels */
    const Adc_ChannelConfigType* Channels;  /**< Pointer to channel config */
} Adc_ConfigType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize ADC driver
 * @param[in] ConfigPtr Pointer to configuration
 */
void Adc_Init(const Adc_ConfigType* ConfigPtr);

/**
 * @brief De-initialize ADC driver
 */
void Adc_DeInit(void);

/**
 * @brief Start group conversion
 * @param[in] Group Group to start
 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/**
 * @brief Stop group conversion
 * @param[in] Group Group to stop
 */
void Adc_StopGroupConversion(Adc_GroupType Group);

/**
 * @brief Read group conversion results
 * @param[in] Group Group to read
 * @param[out] DataBufferPtr Pointer to result buffer
 * @return E_OK if successful
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief Get group status
 * @param[in] Group Group to query
 * @return Current status
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief Enable hardware trigger
 * @param[in] Group Group to enable trigger
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Disable hardware trigger
 * @param[in] Group Group to disable trigger
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief Get version information
 * @param[out] versioninfo Pointer to version info
 */
void Adc_GetVersionInfo(Std_VersionInfoType* versioninfo);

/*============================================================================*
 * SIMULATION FUNCTIONS (FOR TESTING)
 *============================================================================*/

/**
 * @brief Set simulated ADC value
 * @param[in] Channel Channel to set
 * @param[in] Value Value to simulate
 */
void Adc_SimSetValue(Adc_ChannelType Channel, Adc_ValueGroupType Value);

/**
 * @brief Get current simulated ADC value
 * @param[in] Channel Channel to read
 * @return Current simulated value
 */
Adc_ValueGroupType Adc_SimGetValue(Adc_ChannelType Channel);

/**
 * @brief Trigger conversion complete (for simulation)
 * @param[in] Group Group to complete
 */
void Adc_SimTriggerComplete(Adc_GroupType Group);

#endif /* ADC_H */
