/**
 * @file SwitchEvent.h
 * @brief SwitchEvent SWC Interface
 * @details Receives CAN light switch signals and validates them with E2E protection
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [FunSafReq01-01] Light request plausibility check
 */

#ifndef SWITCHEVENT_H
#define SWITCHEVENT_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte/Rte_SwitchEvent.h"
#include "BSW/E2E/E2E_P01.h"
#include "FLM_Config.h"

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Light switch message timeout (cycles at 10ms) */
#define SWITCHEVENT_TIMEOUT_CYCLES      (FLM_CAN_TIMEOUT_MS / FLM_MAIN_FUNCTION_PERIOD_MS)

/** @brief E2E timeout cycles */
#define SWITCHEVENT_E2E_TIMEOUT_CYCLES  (FLM_E2E_TIMEOUT_MS / FLM_MAIN_FUNCTION_PERIOD_MS)

/** @brief Maximum E2E error count before failure */
#define SWITCHEVENT_MAX_E2E_ERRORS      5U

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief SwitchEvent internal state
 */
typedef struct {
    /* Initialization */
    boolean isInitialized;

    /* E2E protection state */
    E2E_P01ConfigType e2eConfig;
    E2E_P01CheckStateType e2eCheckState;
    E2E_SMCheckStateType e2eSmState;
    E2E_SMConfigType e2eSmConfig;

    /* Light switch status */
    LightSwitchStatus lightSwitchStatus;
    E2E_P01CheckStatusType e2eStatus;
    E2E_SMStateType e2eSmStatus;

    /* Timing */
    uint16_t timeoutCounter;
    uint16_t e2eTimeoutCounter;
    uint32_t lastValidTimestamp;
    uint32_t currentTimestamp;

    /* Error tracking */
    uint8_t consecutiveE2EErrors;
    uint8_t consecutiveTimeouts;
    boolean e2eFailureActive;
    boolean timeoutActive;

    /* Message data */
    uint8_t lastMessageData[FLM_CAN_LIGHTSWITCH_MSG_LEN];
    boolean newMessageReceived;
} SwitchEvent_StateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize SwitchEvent component
 * @details Called once at ECU startup
 */
void SwitchEvent_Init(void);

/**
 * @brief Main function for SwitchEvent
 * @details Called every 10ms
 *          [FunSafReq01-01] Performs E2E check
 *          [SysSafReq01] CAN timeout detection
 *          [SysSafReq03] Reports checkpoint to WdgM
 */
void SwitchEvent_MainFunction(void);

/**
 * @brief Get current E2E status
 * @return Current E2E check status
 */
E2E_P01CheckStatusType SwitchEvent_GetE2EStatus(void);

/**
 * @brief Get current light switch request
 * @return Current light switch status with validity
 */
LightSwitchStatus SwitchEvent_GetLightRequest(void);

/**
 * @brief Get E2E state machine status
 * @return Current E2E state machine status
 */
E2E_SMStateType SwitchEvent_GetE2ESmStatus(void);

/**
 * @brief Check if timeout is active
 * @return TRUE if timeout is active
 */
boolean SwitchEvent_IsTimeoutActive(void);

/**
 * @brief Process received CAN message
 * @details Called by COM layer when new message received
 * @param[in] data Pointer to message data
 * @param[in] length Message length
 */
void SwitchEvent_ProcessCanMessage(const uint8_t* data, uint8_t length);

/**
 * @brief Get component state (for testing)
 * @return Pointer to internal state
 */
const SwitchEvent_StateType* SwitchEvent_GetState(void);

#endif /* SWITCHEVENT_H */
