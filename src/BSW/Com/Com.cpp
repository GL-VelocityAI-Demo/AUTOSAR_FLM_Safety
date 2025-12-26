/**
 * @file Com.cpp
 * @brief AUTOSAR COM Module Implementation
 * @details Communication module for signal-based communication
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Com.h"
#include "Application/SwitchEvent/SwitchEvent.h"
#include <cstring>

/*============================================================================*
 * LOCAL TYPES
 *============================================================================*/

/**
 * @brief I-PDU runtime data
 */
typedef struct {
    uint8_t data[8];
    uint8_t length;
    boolean newData;
    uint32_t rxTimestamp;
    uint16_t timeoutCounter;
} Com_IpduDataType;

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean Com_Initialized = FALSE;

/** @brief I-PDU runtime data */
static Com_IpduDataType Com_IpduData[COM_MAX_IPDU_COUNT];

/** @brief Signal data storage */
static uint32_t Com_SignalData[COM_MAX_SIGNAL_COUNT];

/** @brief RX timeout callback enabled */
static boolean Com_TimeoutEnabled = TRUE;

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize COM module
 */
void Com_Init(void) {
    uint8_t i;

    /* Initialize I-PDU data */
    for (i = 0U; i < COM_MAX_IPDU_COUNT; i++) {
        (void)memset(Com_IpduData[i].data, 0, sizeof(Com_IpduData[i].data));
        Com_IpduData[i].length = 0U;
        Com_IpduData[i].newData = FALSE;
        Com_IpduData[i].rxTimestamp = 0U;
        Com_IpduData[i].timeoutCounter = 0U;
    }

    /* Initialize signal data */
    for (i = 0U; i < COM_MAX_SIGNAL_COUNT; i++) {
        Com_SignalData[i] = 0U;
    }

    Com_Initialized = TRUE;
}

/**
 * @brief De-initialize COM module
 */
void Com_DeInit(void) {
    Com_Initialized = FALSE;
}

/**
 * @brief Main function for RX processing
 */
void Com_MainFunctionRx(void) {
    uint8_t i;

    if (!Com_Initialized) {
        return;
    }

    /* Process received data and update timeout counters */
    for (i = 0U; i < COM_NUM_IPDUS; i++) {
        if (Com_IpduData[i].newData) {
            /* Reset timeout counter on new data */
            Com_IpduData[i].timeoutCounter = 0U;
            Com_IpduData[i].newData = FALSE;

            /* Forward to SwitchEvent for light switch message */
            if (i == COM_IPDU_LIGHTSWITCH_RX) {
                SwitchEvent_ProcessCanMessage(
                    Com_IpduData[i].data,
                    Com_IpduData[i].length
                );
            }
        } else {
            /* Increment timeout counter */
            if (Com_TimeoutEnabled) {
                Com_IpduData[i].timeoutCounter++;
            }
        }
    }
}

/**
 * @brief Main function for TX processing
 */
void Com_MainFunctionTx(void) {
    if (!Com_Initialized) {
        return;
    }

    /* TX processing would happen here */
}

/**
 * @brief Send signal
 */
Std_ReturnType Com_SendSignal(uint16_t SignalId, const void* SignalDataPtr) {
    if (!Com_Initialized) {
        return E_NOT_OK;
    }

    if ((SignalId >= COM_MAX_SIGNAL_COUNT) || (SignalDataPtr == NULL_PTR)) {
        return E_NOT_OK;
    }

    /* Store signal data (simplified - assumes 32-bit signals) */
    Com_SignalData[SignalId] = *static_cast<const uint32_t*>(SignalDataPtr);

    return E_OK;
}

/**
 * @brief Receive signal
 */
Std_ReturnType Com_ReceiveSignal(uint16_t SignalId, void* SignalDataPtr) {
    if (!Com_Initialized) {
        return E_NOT_OK;
    }

    if ((SignalId >= COM_MAX_SIGNAL_COUNT) || (SignalDataPtr == NULL_PTR)) {
        return E_NOT_OK;
    }

    /* Return stored signal data */
    *static_cast<uint32_t*>(SignalDataPtr) = Com_SignalData[SignalId];

    return E_OK;
}

/**
 * @brief RX indication callback
 */
void Com_RxIndication(PduIdType PduId, const PduInfoType* PduInfoPtr) {
    if (!Com_Initialized) {
        return;
    }

    if ((PduId >= COM_MAX_IPDU_COUNT) || (PduInfoPtr == NULL_PTR)) {
        return;
    }

    if (PduInfoPtr->SduDataPtr == NULL_PTR) {
        return;
    }

    /* Store received data */
    uint8_t length = (PduInfoPtr->SduLength > 8U) ? 8U :
                     static_cast<uint8_t>(PduInfoPtr->SduLength);

    (void)memcpy(Com_IpduData[PduId].data, PduInfoPtr->SduDataPtr, length);
    Com_IpduData[PduId].length = length;
    Com_IpduData[PduId].newData = TRUE;
}

/**
 * @brief TX confirmation callback
 */
void Com_TxConfirmation(PduIdType TxPduId) {
    STD_UNUSED(TxPduId);

    if (!Com_Initialized) {
        return;
    }

    /* TX confirmation handling */
}

/**
 * @brief Trigger I-PDU send
 */
Std_ReturnType Com_TriggerIPDUSend(PduIdType PduId) {
    if (!Com_Initialized) {
        return E_NOT_OK;
    }

    if (PduId >= COM_MAX_IPDU_COUNT) {
        return E_NOT_OK;
    }

    /* Would trigger actual transmission */

    return E_OK;
}

/**
 * @brief Start I-PDU group
 */
void Com_IpduGroupStart(uint16_t IpduGroupId) {
    STD_UNUSED(IpduGroupId);

    /* Enable I-PDU group */
}

/**
 * @brief Stop I-PDU group
 */
void Com_IpduGroupStop(uint16_t IpduGroupId) {
    STD_UNUSED(IpduGroupId);

    /* Disable I-PDU group */
}

/**
 * @brief Get version information
 */
void Com_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) {
        return;
    }

    VersionInfo->vendorID = 0U;
    VersionInfo->moduleID = 50U;  /* COM module ID */
    VersionInfo->sw_major_version = 1U;
    VersionInfo->sw_minor_version = 0U;
    VersionInfo->sw_patch_version = 0U;
}
