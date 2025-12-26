/**
 * @file BswM.h
 * @brief AUTOSAR BSW Mode Manager Interface
 * @details BSW Mode Manager for mode arbitration and management
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef BSWM_H
#define BSWM_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "ComStack_Types.h"

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief BswM mode type
 */
typedef enum {
    BSWM_MODE_STARTUP       = 0x00U,
    BSWM_MODE_RUN           = 0x01U,
    BSWM_MODE_POST_RUN      = 0x02U,
    BSWM_MODE_SHUTDOWN      = 0x03U,
    BSWM_MODE_SLEEP         = 0x04U
} BswM_ModeType;

/**
 * @brief BswM configuration type
 */
typedef struct {
    uint8_t numModes;
} BswM_ConfigType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize BswM module
 * @param[in] ConfigPtr Pointer to configuration
 */
void BswM_Init(const BswM_ConfigType* ConfigPtr);

/**
 * @brief De-initialize BswM module
 */
void BswM_Deinit(void);

/**
 * @brief Main function for BswM
 */
void BswM_MainFunction(void);

/**
 * @brief Request mode
 * @param[in] requesting_user User requesting mode
 * @param[in] requested_mode Requested mode
 * @return E_OK on success
 */
Std_ReturnType BswM_RequestMode(uint16_t requesting_user, BswM_ModeType requested_mode);

/**
 * @brief Get current mode
 * @param[out] current_mode Pointer to receive mode
 * @return E_OK on success
 */
Std_ReturnType BswM_GetCurrentMode(BswM_ModeType* current_mode);

/**
 * @brief Communication allowed indication
 * @param[in] Network Network handle
 * @param[in] allowed TRUE if allowed
 */
void BswM_ComM_CurrentMode(NetworkHandleType Network, ComM_ModeType RequestedMode);

/**
 * @brief ECU state changed indication
 * @param[in] state New state
 */
void BswM_EcuM_CurrentState(uint8_t state);

/**
 * @brief Request ECU reset
 */
void BswM_RequestReset(void);

/**
 * @brief Get version information
 * @param[out] VersionInfo Pointer to version info
 */
void BswM_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* BSWM_H */
