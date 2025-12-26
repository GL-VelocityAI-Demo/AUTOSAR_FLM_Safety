/**
 * @file WdgM.h
 * @brief AUTOSAR Watchdog Manager Interface
 * @details Implements alive, deadline, and logical supervision
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq03] Watchdog supervision
 */

#ifndef WDGM_H
#define WDGM_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "Rte/Rte_Type.h"
#include "WdgM_Cfg.h"

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief Supervised entity runtime data
 */
typedef struct {
    WdgM_LocalStatusType localStatus;
    uint16_t aliveCounter;
    uint16_t aliveIndicationsInCycle;
    uint32_t lastCheckpointTime;
    uint8_t failedCycleCount;
    boolean isActive;
} WdgM_SupervisedEntityRuntimeType;

/**
 * @brief WdgM configuration type
 */
typedef struct {
    uint8_t numSupervisedEntities;
    uint16_t supervisionCycleMs;
    uint8_t failedRefCycles;
} WdgM_ConfigType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize Watchdog Manager
 * @param[in] ConfigPtr Pointer to configuration
 */
void WdgM_Init(const WdgM_ConfigType* ConfigPtr);

/**
 * @brief De-initialize Watchdog Manager
 */
void WdgM_DeInit(void);

/**
 * @brief Main function for WdgM
 * @details Called every 5ms to perform supervision
 */
void WdgM_MainFunction(void);

/**
 * @brief Report checkpoint reached
 * @details [SysSafReq03] Program flow monitoring
 * @param[in] SEId Supervised Entity ID
 * @param[in] CPId Checkpoint ID
 * @return E_OK on success
 */
Std_ReturnType WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
);

/**
 * @brief Update alive counter
 * @param[in] SEId Supervised Entity ID
 * @return E_OK on success
 */
Std_ReturnType WdgM_UpdateAliveCounter(WdgM_SupervisedEntityIdType SEId);

/**
 * @brief Get global supervision status
 * @param[out] Status Pointer to receive status
 * @return E_OK on success
 */
Std_ReturnType WdgM_GetGlobalStatus(WdgM_GlobalStatusType* Status);

/**
 * @brief Get local supervision status
 * @param[in] SEId Supervised Entity ID
 * @param[out] Status Pointer to receive status
 * @return E_OK on success
 */
Std_ReturnType WdgM_GetLocalStatus(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_LocalStatusType* Status
);

/**
 * @brief Set mode
 * @param[in] Mode New mode
 * @return E_OK on success
 */
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode);

/**
 * @brief Get mode
 * @param[out] Mode Pointer to receive mode
 * @return E_OK on success
 */
Std_ReturnType WdgM_GetMode(WdgM_ModeType* Mode);

/**
 * @brief Get version information
 * @param[out] VersionInfo Pointer to version info
 */
void WdgM_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**
 * @brief Perform reset (for testing)
 */
void WdgM_PerformReset(void);

#endif /* WDGM_H */
