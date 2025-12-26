/**
 * @file SafetyMonitor.h
 * @brief SafetyMonitor SWC Interface
 * @details Aggregates safety status and triggers safe state
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [ECU17] FTTI monitoring, [FunSafReq01-03] Safe state control
 */

#ifndef SAFETYMONITOR_H
#define SAFETYMONITOR_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Rte/Rte_SafetyMonitor.h"
#include "FLM_Config.h"

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief SafetyMonitor internal state
 */
typedef struct {
    /* Initialization */
    boolean isInitialized;
    boolean inSafeState;

    /* Component status tracking */
    boolean switchEventFault;
    boolean lightRequestFault;
    boolean flmFault;
    boolean headlightFault;
    boolean wdgmFault;

    /* E2E monitoring */
    E2E_P01CheckStatusType e2eStatus;
    E2E_SMStateType e2eSmStatus;
    uint32_t e2eFailureStartTime;
    boolean e2eTimeoutActive;

    /* Headlight status */
    HeadlightFaultStatus headlightStatus;

    /* FLM state */
    FLM_StateType flmState;

    /* WdgM status */
    WdgM_GlobalStatusType wdgmGlobalStatus;

    /* Fault aggregation */
    uint8_t totalFaultCount;
    uint32_t firstFaultTime;
    boolean fttiActive;

    /* Safe state */
    SafeStateReason safeStateReason;
    uint32_t safeStateEntryTime;
    HeadlightCommand safeStateCommand;

    /* Ambient light for safe state decision */
    AmbientLightLevel lastAmbientLight;
    boolean isDaytime;

    /* Global status */
    SafetyStatusType globalStatus;

    /* Timing */
    uint32_t currentTime;
} SafetyMonitor_StateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize SafetyMonitor component
 * @details Called once at ECU startup
 */
void SafetyMonitor_Init(void);

/**
 * @brief Main function for SafetyMonitor
 * @details Called every 5ms
 *          [ECU17] FTTI monitoring (200ms)
 */
void SafetyMonitor_MainFunction(void);

/**
 * @brief Get global safety status
 * @return Current global safety status
 */
SafetyStatusType SafetyMonitor_GetGlobalStatus(void);

/**
 * @brief Trigger safe state transition
 * @param[in] reason Reason for safe state entry
 */
void SafetyMonitor_TriggerSafeState(SafeStateReason reason);

/**
 * @brief Check if in safe state
 * @return TRUE if in safe state
 */
boolean SafetyMonitor_IsInSafeState(void);

/**
 * @brief Get safe state reason
 * @return Reason for current safe state
 */
SafeStateReason SafetyMonitor_GetSafeStateReason(void);

/**
 * @brief Get safe state headlight command
 * @return Headlight command for safe state
 */
HeadlightCommand SafetyMonitor_GetSafeStateCommand(void);

/**
 * @brief Get component state (for testing)
 * @return Pointer to internal state
 */
const SafetyMonitor_StateType* SafetyMonitor_GetState(void);

#endif /* SAFETYMONITOR_H */
