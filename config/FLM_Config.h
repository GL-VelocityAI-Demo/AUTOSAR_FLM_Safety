/**
 * @file FLM_Config.h
 * @brief FLM System Configuration
 * @details Configuration parameters for the Front Light Management system
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef FLM_CONFIG_H
#define FLM_CONFIG_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"

/*============================================================================*
 * TASK SCHEDULING CONFIGURATION
 *============================================================================*/

/** @brief Main function period for FLM Application (ms) */
#define FLM_MAIN_FUNCTION_PERIOD_MS         10U

/** @brief Ambient light sensor reading period (ms) */
#define FLM_AMBIENT_LIGHT_PERIOD_MS         20U

/** @brief Safety monitor check period (ms) */
#define FLM_SAFETY_MONITOR_PERIOD_MS        5U

/** @brief Watchdog manager period (ms) */
#define FLM_WDGM_PERIOD_MS                  5U

/*============================================================================*
 * TIMING THRESHOLDS - Safety Requirements
 *============================================================================*/

/** @brief CAN message timeout threshold (ms) [SysSafReq01] */
#define FLM_CAN_TIMEOUT_MS                  50U

/** @brief E2E protection timeout (ms) [SysSafReq02] */
#define FLM_E2E_TIMEOUT_MS                  100U

/** @brief Fault Tolerant Time Interval (ms) [ECU17] */
#define FLM_FTTI_MS                         200U

/** @brief Safe state transition time (ms) [FunSafReq01-03] */
#define FLM_SAFE_STATE_TRANSITION_MS        100U

/*============================================================================*
 * AMBIENT LIGHT SENSOR CONFIGURATION
 *============================================================================*/

/** @brief Threshold for lights ON (ADC value ~200 lux) */
#define FLM_AMBIENT_THRESHOLD_ON            800U

/** @brief Threshold for lights OFF with hysteresis (ADC value ~250 lux) */
#define FLM_AMBIENT_THRESHOLD_OFF           1000U

/** @brief Maximum ADC change per 100ms for plausibility [FunSafReq01-02] */
#define FLM_AMBIENT_RATE_LIMIT              500U

/** @brief ADC value threshold for open circuit detection */
#define FLM_AMBIENT_OPEN_CIRCUIT            100U

/** @brief ADC value threshold for short circuit detection */
#define FLM_AMBIENT_SHORT_CIRCUIT           3995U

/** @brief Number of ADC samples for averaging */
#define FLM_ADC_SAMPLES                     4U

/** @brief ADC resolution in bits */
#define FLM_ADC_RESOLUTION                  12U

/** @brief Maximum ADC value based on resolution */
#define FLM_ADC_MAX_VALUE                   ((1U << FLM_ADC_RESOLUTION) - 1U)

/*============================================================================*
 * HEADLIGHT CONFIGURATION
 *============================================================================*/

/** @brief Fault detection time (ms) [SysSafReq10] */
#define FLM_HEADLIGHT_FAULT_DETECT_MS       20U

/** @brief Minimum current for ON detection (mA) */
#define FLM_HEADLIGHT_MIN_CURRENT_MA        100U

/** @brief Maximum current threshold (mA) */
#define FLM_HEADLIGHT_MAX_CURRENT_MA        15000U

/** @brief Open load current threshold (mA) */
#define FLM_HEADLIGHT_OPEN_LOAD_MA          50U

/** @brief ADC to current conversion factor (mA per ADC count) */
#define FLM_HEADLIGHT_CURRENT_FACTOR        10U

/*============================================================================*
 * E2E PROFILE 01 CONFIGURATION
 *============================================================================*/

/** @brief Data length for light switch message (bits) */
#define FLM_E2E_LIGHTSWITCH_DATA_LENGTH     32U

/** @brief Data ID for light switch message */
#define FLM_E2E_LIGHTSWITCH_DATA_ID         0x0100U

/** @brief Maximum delta counter for E2E Profile 01 */
#define FLM_E2E_MAX_DELTA_COUNTER           2U

/** @brief Maximum cycles without new/repeated data */
#define FLM_E2E_MAX_NO_NEW_DATA             5U

/** @brief Sync counter for E2E state machine */
#define FLM_E2E_SYNC_COUNTER                2U

/** @brief Counter offset in bits */
#define FLM_E2E_COUNTER_OFFSET              8U

/** @brief CRC offset in bits */
#define FLM_E2E_CRC_OFFSET                  0U

/*============================================================================*
 * CAN CONFIGURATION
 *============================================================================*/

/** @brief CAN message ID for light switch command */
#define FLM_CAN_LIGHTSWITCH_MSG_ID          0x200U

/** @brief CAN message length for light switch (bytes) */
#define FLM_CAN_LIGHTSWITCH_MSG_LEN         4U

/** @brief CAN controller ID */
#define FLM_CAN_CONTROLLER_ID               0U

/*============================================================================*
 * DIO CONFIGURATION
 *============================================================================*/

/** @brief DIO channel for low beam output */
#define FLM_DIO_CHANNEL_LOW_BEAM            0U

/** @brief DIO channel for high beam output */
#define FLM_DIO_CHANNEL_HIGH_BEAM           1U

/** @brief DIO channel for headlight feedback */
#define FLM_DIO_CHANNEL_FEEDBACK            2U

/*============================================================================*
 * ADC CONFIGURATION
 *============================================================================*/

/** @brief ADC channel for ambient light sensor */
#define FLM_ADC_CHANNEL_AMBIENT             0U

/** @brief ADC channel for current sense */
#define FLM_ADC_CHANNEL_CURRENT             1U

/** @brief ADC conversion time (microseconds) */
#define FLM_ADC_CONVERSION_TIME_US          10U

/*============================================================================*
 * WATCHDOG MANAGER CONFIGURATION
 *============================================================================*/

/** @brief Number of supervised entities */
#define FLM_WDGM_NUM_ENTITIES               5U

/** @brief Supervision reference cycle (ms) */
#define FLM_WDGM_SUPERVISION_CYCLE_MS       100U

/** @brief Alive supervision margin (min) */
#define FLM_WDGM_ALIVE_MARGIN_MIN           8U

/** @brief Alive supervision margin (max) */
#define FLM_WDGM_ALIVE_MARGIN_MAX           12U

/** @brief Failed supervision reference counter */
#define FLM_WDGM_FAILED_REF_CYCLE           3U

/*============================================================================*
 * ERROR HANDLING CONFIGURATION
 *============================================================================*/

/** @brief Maximum consecutive errors before degraded mode */
#define FLM_MAX_CONSECUTIVE_ERRORS          3U

/** @brief Maximum total faults before safe state */
#define FLM_MAX_TOTAL_FAULTS                5U

/** @brief Error debounce count */
#define FLM_ERROR_DEBOUNCE_COUNT            2U

/*============================================================================*
 * SYSTEM CONFIGURATION
 *============================================================================*/

/** @brief System tick period (ms) */
#define FLM_SYSTEM_TICK_MS                  1U

/** @brief Enable development error detection */
#define FLM_DEV_ERROR_DETECT                STD_ON

/** @brief Enable version info API */
#define FLM_VERSION_INFO_API                STD_ON

/** @brief Enable debug output */
#define FLM_DEBUG_OUTPUT                    STD_OFF

/*============================================================================*
 * COMPILE-TIME CHECKS
 *============================================================================*/

/* Verify timing constraints */
STD_STATIC_ASSERT(FLM_SAFE_STATE_TRANSITION_MS <= FLM_FTTI_MS,
                  "Safe state transition must be within FTTI");

STD_STATIC_ASSERT(FLM_E2E_TIMEOUT_MS < FLM_FTTI_MS,
                  "E2E timeout must be less than FTTI");

STD_STATIC_ASSERT(FLM_CAN_TIMEOUT_MS < FLM_E2E_TIMEOUT_MS,
                  "CAN timeout must be less than E2E timeout");

/* Verify hysteresis configuration */
STD_STATIC_ASSERT(FLM_AMBIENT_THRESHOLD_ON < FLM_AMBIENT_THRESHOLD_OFF,
                  "ON threshold must be less than OFF threshold");

/* Verify ADC configuration */
STD_STATIC_ASSERT(FLM_AMBIENT_OPEN_CIRCUIT < FLM_AMBIENT_SHORT_CIRCUIT,
                  "Open circuit threshold must be less than short circuit");

#endif /* FLM_CONFIG_H */
