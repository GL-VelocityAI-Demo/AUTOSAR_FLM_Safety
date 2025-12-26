/**
 * @file Dem.h
 * @brief AUTOSAR Diagnostic Event Manager Interface
 * @details DEM module for diagnostic event and DTC management
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef DEM_H
#define DEM_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "Rte/Rte_Type.h"
#include "Dem_Cfg.h"

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize DEM module
 */
void Dem_Init(void);

/**
 * @brief Pre-initialize DEM module
 */
void Dem_PreInit(void);

/**
 * @brief Shutdown DEM module
 */
void Dem_Shutdown(void);

/**
 * @brief Main function for DEM
 */
void Dem_MainFunction(void);

/**
 * @brief Set event status
 * @param[in] EventId Event identifier
 * @param[in] EventStatus Event status (passed/failed)
 * @return E_OK on success
 */
Std_ReturnType Dem_SetEventStatus(
    DEM_EventIdType EventId,
    Dem_EventStatusType EventStatus
);

/**
 * @brief Get event status
 * @param[in] EventId Event identifier
 * @param[out] EventStatus Pointer to receive status
 * @return E_OK on success
 */
Std_ReturnType Dem_GetEventStatus(
    DEM_EventIdType EventId,
    Dem_UdsStatusByteType* EventStatus
);

/**
 * @brief Reset event status
 * @param[in] EventId Event identifier
 * @return E_OK on success
 */
Std_ReturnType Dem_ResetEventStatus(DEM_EventIdType EventId);

/**
 * @brief Clear DTC
 * @param[in] DTC DTC value (or group)
 * @return E_OK on success
 */
Std_ReturnType Dem_ClearDTC(uint32_t DTC);

/**
 * @brief Get DTC status
 * @param[in] DTC DTC value
 * @param[out] Status Pointer to receive status
 * @return E_OK on success
 */
Std_ReturnType Dem_GetDTCStatus(uint32_t DTC, Dem_UdsStatusByteType* Status);

/**
 * @brief Report operation cycle state
 * @param[in] OperationCycleId Operation cycle ID
 * @param[in] CycleState Cycle state (start/end)
 * @return E_OK on success
 */
Std_ReturnType Dem_SetOperationCycleState(
    Dem_OperationCycleIdType OperationCycleId,
    uint8_t CycleState
);

/**
 * @brief Enable DTC setting
 */
Std_ReturnType Dem_EnableDTCSetting(void);

/**
 * @brief Disable DTC setting
 */
Std_ReturnType Dem_DisableDTCSetting(void);

/**
 * @brief Get number of stored events
 * @param[out] NumberOfEvents Pointer to receive count
 * @return E_OK on success
 */
Std_ReturnType Dem_GetNumberOfEvents(uint16_t* NumberOfEvents);

/**
 * @brief Get version information
 * @param[out] VersionInfo Pointer to version info
 */
void Dem_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* DEM_H */
