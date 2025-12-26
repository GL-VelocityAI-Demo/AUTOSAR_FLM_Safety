/**
 * @file Headlight.h
 * @brief Headlight SWC Interface
 * @details Controls headlight output with feedback monitoring
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq10] Output stage diagnosis
 */

#ifndef HEADLIGHT_H
#define HEADLIGHT_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte/Rte_Headlight.h"
#include "FLM_Config.h"

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Fault confirmation cycles (at 10ms) */
#define HEADLIGHT_FAULT_CONFIRM_CYCLES      (FLM_HEADLIGHT_FAULT_DETECT_MS / FLM_MAIN_FUNCTION_PERIOD_MS)

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief Headlight internal state
 */
typedef struct {
    /* Initialization */
    boolean isInitialized;

    /* Command and output */
    HeadlightCommand currentCommand;
    HeadlightCommand requestedCommand;
    boolean lowBeamOutput;
    boolean highBeamOutput;

    /* Feedback monitoring */
    uint16_t feedbackCurrent;       /**< Current sense in mA */
    boolean feedbackState;          /**< Actual current flow detected */

    /* Fault detection [SysSafReq10] */
    HeadlightFaultStatus faultStatus;
    uint8_t openLoadCounter;
    uint8_t shortCircuitCounter;
    uint32_t faultDetectStartTime;
    boolean faultConfirmed;

    /* Timing */
    uint32_t commandChangeTime;
    uint32_t currentTime;
} Headlight_StateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize Headlight component
 * @details Called once at ECU startup
 */
void Headlight_Init(void);

/**
 * @brief Main function for Headlight
 * @details Called every 10ms
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
 * @return TRUE if headlight is ON (low or high beam)
 */
boolean Headlight_GetActualState(void);

/**
 * @brief Get current command
 * @return Current headlight command
 */
HeadlightCommand Headlight_GetCurrentCommand(void);

/**
 * @brief Set simulated feedback current (for testing)
 * @param[in] current Current in mA
 */
void Headlight_SimSetFeedbackCurrent(uint16_t current);

/**
 * @brief Get component state (for testing)
 * @return Pointer to internal state
 */
const Headlight_StateType* Headlight_GetState(void);

#endif /* HEADLIGHT_H */
