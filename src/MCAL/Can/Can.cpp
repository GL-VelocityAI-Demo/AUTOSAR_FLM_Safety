/**
 * @file Can.cpp
 * @brief AUTOSAR CAN Driver Implementation
 * @details MCAL CAN driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Can.h"
#include <cstring>

/*============================================================================*
 * LOCAL TYPES
 *============================================================================*/

/**
 * @brief RX message buffer entry
 */
typedef struct {
    Can_IdType canId;
    uint8_t dlc;
    uint8_t data[CAN_MAX_DATA_LENGTH];
    boolean used;
} Can_RxBufferEntryType;

/**
 * @brief TX message buffer entry
 */
typedef struct {
    PduIdType pduId;
    Can_IdType canId;
    uint8_t dlc;
    uint8_t data[CAN_MAX_DATA_LENGTH];
    boolean pending;
} Can_TxBufferEntryType;

/**
 * @brief Controller state structure
 */
typedef struct {
    Can_ControllerStateType state;
    Can_ErrorStateType errorState;
    boolean interruptsEnabled;
    boolean busOffPending;
} Can_ControllerStateStruct;

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean Can_Initialized = FALSE;

/** @brief Configuration pointer */
static const Can_ConfigType* Can_ConfigPtr = NULL_PTR;

/** @brief Controller states */
static Can_ControllerStateStruct Can_ControllerStates[CAN_NUM_CONTROLLERS];

/** @brief RX buffer */
static Can_RxBufferEntryType Can_RxBuffer[CAN_RX_FIFO_SIZE];
static uint8_t Can_RxBufferHead = 0U;
static uint8_t Can_RxBufferTail = 0U;
static uint8_t Can_RxBufferCount = 0U;

/** @brief TX buffer */
static Can_TxBufferEntryType Can_TxBuffer[CAN_TX_BUFFER_SIZE];
static uint8_t Can_TxBufferCount = 0U;

/** @brief Last transmitted message (for simulation) */
static Can_TxBufferEntryType Can_LastTxMessage;
static boolean Can_LastTxMessageValid = FALSE;

/** @brief TX message counter */
static uint32_t Can_TxCounter = 0U;

/** @brief Callbacks */
static Can_RxIndicationFctType Can_RxIndicationCallback = NULL_PTR;
static Can_TxConfirmationFctType Can_TxConfirmationCallback = NULL_PTR;
static Can_ControllerBusOffFctType Can_BusOffCallback = NULL_PTR;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void Can_ProcessRxBuffer(void);
static void Can_ProcessTxBuffer(void);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize CAN driver
 */
void Can_Init(const Can_ConfigType* Config) {
    uint8_t i;

    if (Config == NULL_PTR) {
        return;
    }

    Can_ConfigPtr = Config;

    /* Initialize controller states */
    for (i = 0U; i < CAN_NUM_CONTROLLERS; i++) {
        Can_ControllerStates[i].state = CAN_CS_STOPPED;
        Can_ControllerStates[i].errorState = CAN_ERRORSTATE_ACTIVE;
        Can_ControllerStates[i].interruptsEnabled = FALSE;
        Can_ControllerStates[i].busOffPending = FALSE;
    }

    /* Clear RX buffer */
    Can_RxBufferHead = 0U;
    Can_RxBufferTail = 0U;
    Can_RxBufferCount = 0U;
    for (i = 0U; i < CAN_RX_FIFO_SIZE; i++) {
        Can_RxBuffer[i].used = FALSE;
    }

    /* Clear TX buffer */
    Can_TxBufferCount = 0U;
    for (i = 0U; i < CAN_TX_BUFFER_SIZE; i++) {
        Can_TxBuffer[i].pending = FALSE;
    }

    Can_LastTxMessageValid = FALSE;
    Can_TxCounter = 0U;

    Can_Initialized = TRUE;
}

/**
 * @brief De-initialize CAN driver
 */
void Can_DeInit(void) {
    Can_ConfigPtr = NULL_PTR;
    Can_Initialized = FALSE;
}

/**
 * @brief Set CAN controller mode
 */
Can_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ModeType Transition) {
    if (!Can_Initialized) {
        return CAN_NOT_OK;
    }

    if (Controller >= CAN_NUM_CONTROLLERS) {
        return CAN_NOT_OK;
    }

    switch (Transition) {
        case CAN_MODE_START:
            Can_ControllerStates[Controller].state = CAN_CS_STARTED;
            Can_ControllerStates[Controller].errorState = CAN_ERRORSTATE_ACTIVE;
            break;

        case CAN_MODE_STOP:
            Can_ControllerStates[Controller].state = CAN_CS_STOPPED;
            break;

        case CAN_MODE_SLEEP:
            Can_ControllerStates[Controller].state = CAN_CS_SLEEP;
            break;

        default:
            return CAN_NOT_OK;
    }

    return CAN_OK;
}

/**
 * @brief Get CAN controller error state
 */
Std_ReturnType Can_GetControllerErrorState(uint8_t ControllerId,
                                            Can_ErrorStateType* ErrorStatePtr) {
    if (!Can_Initialized) {
        return E_NOT_OK;
    }

    if ((ControllerId >= CAN_NUM_CONTROLLERS) || (ErrorStatePtr == NULL_PTR)) {
        return E_NOT_OK;
    }

    *ErrorStatePtr = Can_ControllerStates[ControllerId].errorState;

    return E_OK;
}

/**
 * @brief Get CAN controller mode
 */
Std_ReturnType Can_GetControllerMode(uint8_t Controller,
                                      Can_ControllerStateType* ControllerModePtr) {
    if (!Can_Initialized) {
        return E_NOT_OK;
    }

    if ((Controller >= CAN_NUM_CONTROLLERS) || (ControllerModePtr == NULL_PTR)) {
        return E_NOT_OK;
    }

    *ControllerModePtr = Can_ControllerStates[Controller].state;

    return E_OK;
}

/**
 * @brief Write CAN message
 */
Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo) {
    uint8_t i;

    STD_UNUSED(Hth);

    if (!Can_Initialized) {
        return CAN_NOT_OK;
    }

    if (PduInfo == NULL_PTR) {
        return CAN_NOT_OK;
    }

    if (PduInfo->sdu == NULL_PTR) {
        return CAN_NOT_OK;
    }

    if (Can_TxBufferCount >= CAN_TX_BUFFER_SIZE) {
        return CAN_BUSY;
    }

    /* Find empty slot in TX buffer */
    for (i = 0U; i < CAN_TX_BUFFER_SIZE; i++) {
        if (!Can_TxBuffer[i].pending) {
            Can_TxBuffer[i].pduId = PduInfo->swPduHandle;
            Can_TxBuffer[i].canId = PduInfo->id;
            Can_TxBuffer[i].dlc = PduInfo->length;
            (void)memcpy(Can_TxBuffer[i].data, PduInfo->sdu, PduInfo->length);
            Can_TxBuffer[i].pending = TRUE;
            Can_TxBufferCount++;
            break;
        }
    }

    return CAN_OK;
}

/**
 * @brief Main function for TX processing
 */
void Can_MainFunction_Write(void) {
    if (!Can_Initialized) {
        return;
    }

    Can_ProcessTxBuffer();
}

/**
 * @brief Main function for RX processing
 */
void Can_MainFunction_Read(void) {
    if (!Can_Initialized) {
        return;
    }

    Can_ProcessRxBuffer();
}

/**
 * @brief Main function for bus-off processing
 */
void Can_MainFunction_BusOff(void) {
    uint8_t i;

    if (!Can_Initialized) {
        return;
    }

    for (i = 0U; i < CAN_NUM_CONTROLLERS; i++) {
        if (Can_ControllerStates[i].busOffPending) {
            Can_ControllerStates[i].busOffPending = FALSE;
            Can_ControllerStates[i].errorState = CAN_ERRORSTATE_BUSOFF;

            if (Can_BusOffCallback != NULL_PTR) {
                Can_BusOffCallback(i);
            }
        }
    }
}

/**
 * @brief Main function for wakeup processing
 */
void Can_MainFunction_Wakeup(void) {
    /* Not implemented in simulation */
}

/**
 * @brief Main function for mode transitions
 */
void Can_MainFunction_Mode(void) {
    /* Not implemented in simulation */
}

/**
 * @brief Get version information
 */
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    if (versioninfo == NULL_PTR) {
        return;
    }

    versioninfo->vendorID = 0U;
    versioninfo->moduleID = 80U;  /* CAN module ID */
    versioninfo->sw_major_version = CAN_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = CAN_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = CAN_SW_PATCH_VERSION;
}

/**
 * @brief Enable controller interrupts
 */
void Can_EnableControllerInterrupts(uint8_t Controller) {
    if (Controller < CAN_NUM_CONTROLLERS) {
        Can_ControllerStates[Controller].interruptsEnabled = TRUE;
    }
}

/**
 * @brief Disable controller interrupts
 */
void Can_DisableControllerInterrupts(uint8_t Controller) {
    if (Controller < CAN_NUM_CONTROLLERS) {
        Can_ControllerStates[Controller].interruptsEnabled = FALSE;
    }
}

/**
 * @brief Check wakeup
 */
Std_ReturnType Can_CheckWakeup(uint8_t Controller) {
    STD_UNUSED(Controller);
    return E_NOT_OK;  /* No wakeup in simulation */
}

/*============================================================================*
 * CALLBACK CONFIGURATION
 *============================================================================*/

void Can_SetRxIndicationCallback(Can_RxIndicationFctType callback) {
    Can_RxIndicationCallback = callback;
}

void Can_SetTxConfirmationCallback(Can_TxConfirmationFctType callback) {
    Can_TxConfirmationCallback = callback;
}

void Can_SetControllerBusOffCallback(Can_ControllerBusOffFctType callback) {
    Can_BusOffCallback = callback;
}

/*============================================================================*
 * LOCAL FUNCTIONS
 *============================================================================*/

/**
 * @brief Process RX buffer
 */
static void Can_ProcessRxBuffer(void) {
    while (Can_RxBufferCount > 0U) {
        Can_RxBufferEntryType* entry = &Can_RxBuffer[Can_RxBufferTail];

        if (entry->used && (Can_RxIndicationCallback != NULL_PTR)) {
            Can_RxIndicationCallback(0U, entry->canId, entry->dlc, entry->data);
        }

        entry->used = FALSE;
        Can_RxBufferTail = (Can_RxBufferTail + 1U) % CAN_RX_FIFO_SIZE;
        Can_RxBufferCount--;
    }
}

/**
 * @brief Process TX buffer
 */
static void Can_ProcessTxBuffer(void) {
    uint8_t i;

    for (i = 0U; i < CAN_TX_BUFFER_SIZE; i++) {
        if (Can_TxBuffer[i].pending) {
            /* Store as last TX message */
            Can_LastTxMessage = Can_TxBuffer[i];
            Can_LastTxMessageValid = TRUE;
            Can_TxCounter++;

            /* Call TX confirmation */
            if (Can_TxConfirmationCallback != NULL_PTR) {
                Can_TxConfirmationCallback(Can_TxBuffer[i].pduId);
            }

            Can_TxBuffer[i].pending = FALSE;
            Can_TxBufferCount--;
        }
    }
}

/*============================================================================*
 * SIMULATION FUNCTIONS
 *============================================================================*/

void Can_SimReceiveMessage(uint8_t Controller,
                           Can_IdType CanId,
                           uint8_t Dlc,
                           const uint8_t* Data) {
    STD_UNUSED(Controller);

    if (!Can_Initialized) {
        return;
    }

    if ((Data == NULL_PTR) || (Dlc > CAN_MAX_DATA_LENGTH)) {
        return;
    }

    if (Can_RxBufferCount >= CAN_RX_FIFO_SIZE) {
        return;  /* Buffer full */
    }

    Can_RxBuffer[Can_RxBufferHead].canId = CanId;
    Can_RxBuffer[Can_RxBufferHead].dlc = Dlc;
    (void)memcpy(Can_RxBuffer[Can_RxBufferHead].data, Data, Dlc);
    Can_RxBuffer[Can_RxBufferHead].used = TRUE;

    Can_RxBufferHead = (Can_RxBufferHead + 1U) % CAN_RX_FIFO_SIZE;
    Can_RxBufferCount++;
}

boolean Can_SimGetLastTxMessage(Can_IdType* CanId, uint8_t* Dlc, uint8_t* Data) {
    if (!Can_LastTxMessageValid) {
        return FALSE;
    }

    if (CanId != NULL_PTR) {
        *CanId = Can_LastTxMessage.canId;
    }
    if (Dlc != NULL_PTR) {
        *Dlc = Can_LastTxMessage.dlc;
    }
    if (Data != NULL_PTR) {
        (void)memcpy(Data, Can_LastTxMessage.data, Can_LastTxMessage.dlc);
    }

    return TRUE;
}

void Can_SimClearRxBuffer(uint8_t Controller) {
    uint8_t i;

    STD_UNUSED(Controller);

    for (i = 0U; i < CAN_RX_FIFO_SIZE; i++) {
        Can_RxBuffer[i].used = FALSE;
    }
    Can_RxBufferHead = 0U;
    Can_RxBufferTail = 0U;
    Can_RxBufferCount = 0U;
}

void Can_SimTriggerBusOff(uint8_t Controller) {
    if (Controller < CAN_NUM_CONTROLLERS) {
        Can_ControllerStates[Controller].busOffPending = TRUE;
    }
}

uint32_t Can_SimGetTxCount(void) {
    return Can_TxCounter;
}
