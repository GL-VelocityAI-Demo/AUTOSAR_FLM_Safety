/**
 * @file FLM_Application.h
 * @brief FLM Application SWC Interface
 * @details Main control logic determining headlight activation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - Main FLM control logic
 */

#ifndef FLM_APPLICATION_H
#define FLM_APPLICATION_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte/Rte_FLM.h"
#include "FLM_Config.h"

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief FLM Application internal state
 */
typedef struct {
    /* Initialization */
    boolean isInitialized;

    /* State machine */
    FLM_StateType currentState;
    FLM_StateType previousState;
    uint32_t stateEntryTime;

    /* Input data */
    LightSwitchStatus lightSwitch;
    AmbientLightLevel ambientLight;
    SignalStatus switchSignalStatus;
    SignalStatus ambientSignalStatus;
    E2E_P01CheckStatusType e2eStatus;

    /* Output data */
    HeadlightCommand headlightCommand;

    /* Hysteresis control */
    boolean lightsCurrentlyOn;
    boolean hysteresisActive;

    /* Error tracking */
    uint8_t consecutiveErrors;
    uint32_t e2eErrorStartTime;
    boolean e2eTimeoutActive;
    uint32_t degradedEntryTime;

    /* Timing */
    uint32_t currentTime;
} FLM_Application_StateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize FLM Application component
 * @details Called once at ECU startup
 */
void FLM_Init(void);

/**
 * @brief Main function for FLM Application
 * @details Called every 10ms
 *          Implements main control logic and state machine
 */
void FLM_MainFunction(void);

/**
 * @brief Get current headlight command
 * @return Current headlight command
 */
HeadlightCommand FLM_GetHeadlightCommand(void);

/**
 * @brief Get current FLM state
 * @return Current state machine state
 */
FLM_StateType FLM_GetCurrentState(void);

/**
 * @brief Trigger transition to safe state
 * @param[in] reason Reason for safe state transition
 */
void FLM_TriggerSafeState(SafeStateReason reason);

/**
 * @brief Check if in safe state
 * @return TRUE if in safe state
 */
boolean FLM_IsInSafeState(void);

/**
 * @brief Get component state (for testing)
 * @return Pointer to internal state
 */
const FLM_Application_StateType* FLM_GetState(void);

#endif /* FLM_APPLICATION_H */
