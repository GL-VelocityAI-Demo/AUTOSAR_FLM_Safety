/**
 * @file Com.h
 * @brief AUTOSAR COM Module Interface
 * @details Communication module for signal-based communication
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef COM_H
#define COM_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "ComStack_Types.h"
#include "Com_Cfg.h"

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize COM module
 */
void Com_Init(void);

/**
 * @brief De-initialize COM module
 */
void Com_DeInit(void);

/**
 * @brief Main function for RX processing
 */
void Com_MainFunctionRx(void);

/**
 * @brief Main function for TX processing
 */
void Com_MainFunctionTx(void);

/**
 * @brief Send signal
 * @param[in] SignalId Signal identifier
 * @param[in] SignalDataPtr Pointer to signal data
 * @return E_OK on success
 */
Std_ReturnType Com_SendSignal(uint16_t SignalId, const void* SignalDataPtr);

/**
 * @brief Receive signal
 * @param[in] SignalId Signal identifier
 * @param[out] SignalDataPtr Pointer to receive data
 * @return E_OK on success
 */
Std_ReturnType Com_ReceiveSignal(uint16_t SignalId, void* SignalDataPtr);

/**
 * @brief RX indication callback
 * @param[in] PduId PDU identifier
 * @param[in] PduInfoPtr Pointer to PDU info
 */
void Com_RxIndication(PduIdType PduId, const PduInfoType* PduInfoPtr);

/**
 * @brief TX confirmation callback
 * @param[in] TxPduId PDU identifier
 */
void Com_TxConfirmation(PduIdType TxPduId);

/**
 * @brief Trigger I-PDU send
 * @param[in] PduId PDU identifier
 * @return E_OK on success
 */
Std_ReturnType Com_TriggerIPDUSend(PduIdType PduId);

/**
 * @brief Start I-PDU group
 * @param[in] IpduGroupId I-PDU group identifier
 */
void Com_IpduGroupStart(uint16_t IpduGroupId);

/**
 * @brief Stop I-PDU group
 * @param[in] IpduGroupId I-PDU group identifier
 */
void Com_IpduGroupStop(uint16_t IpduGroupId);

/**
 * @brief Get version information
 * @param[out] VersionInfo Pointer to version info
 */
void Com_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* COM_H */
