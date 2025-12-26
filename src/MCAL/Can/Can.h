/**
 * @file Can.h
 * @brief AUTOSAR CAN Driver Interface
 * @details MCAL CAN driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef CAN_H
#define CAN_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "ComStack_Types.h"

/*============================================================================*
 * VERSION INFORMATION
 *============================================================================*/
#define CAN_AR_RELEASE_MAJOR_VERSION        23
#define CAN_AR_RELEASE_MINOR_VERSION        11

#define CAN_SW_MAJOR_VERSION                1
#define CAN_SW_MINOR_VERSION                0
#define CAN_SW_PATCH_VERSION                0

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Number of CAN controllers */
#define CAN_NUM_CONTROLLERS                 2U

/** @brief Number of hardware object handles */
#define CAN_NUM_HW_OBJECTS                  32U

/** @brief Maximum CAN data length */
#define CAN_MAX_DATA_LENGTH                 8U

/** @brief Enable development error detection */
#define CAN_DEV_ERROR_DETECT                STD_ON

/** @brief RX FIFO size */
#define CAN_RX_FIFO_SIZE                    16U

/** @brief TX buffer size */
#define CAN_TX_BUFFER_SIZE                  8U

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief CAN hardware unit type
 */
typedef uint8_t Can_HwType;

/**
 * @brief CAN interrupt type
 */
typedef enum {
    CAN_INTERRUPT_TX        = 0x01U,    /**< TX interrupt */
    CAN_INTERRUPT_RX        = 0x02U,    /**< RX interrupt */
    CAN_INTERRUPT_ERROR     = 0x04U,    /**< Error interrupt */
    CAN_INTERRUPT_BUSOFF    = 0x08U,    /**< Bus-off interrupt */
    CAN_INTERRUPT_WAKEUP    = 0x10U     /**< Wakeup interrupt */
} Can_InterruptType;

/**
 * @brief CAN mode type
 */
typedef enum {
    CAN_MODE_UNINIT     = 0x00U,    /**< Uninitialized */
    CAN_MODE_STOP       = 0x01U,    /**< Stopped */
    CAN_MODE_START      = 0x02U,    /**< Started */
    CAN_MODE_SLEEP      = 0x03U     /**< Sleep mode */
} Can_ModeType;

/**
 * @brief CAN object handle type (mailbox)
 */
typedef uint16_t Can_HwHandleType;

/**
 * @brief CAN hardware object configuration
 */
typedef struct {
    Can_HwHandleType HwHandle;      /**< Hardware object handle */
    Can_IdType CanId;               /**< CAN identifier */
    uint8_t HwObjectType;           /**< 0=RX, 1=TX */
    uint8_t DataLength;             /**< Data length */
} Can_HwObjectConfigType;

/**
 * @brief CAN controller configuration
 */
typedef struct {
    uint8_t ControllerId;           /**< Controller ID */
    uint32_t Baudrate;              /**< Baudrate in bps */
    uint8_t NumHwObjects;           /**< Number of hardware objects */
    const Can_HwObjectConfigType* HwObjects; /**< HW objects config */
} Can_ControllerConfigType;

/**
 * @brief CAN driver configuration
 */
typedef struct {
    uint8_t NumControllers;         /**< Number of controllers */
    const Can_ControllerConfigType* Controllers; /**< Controllers config */
} Can_ConfigType;

/**
 * @brief RX indication callback type
 */
typedef void (*Can_RxIndicationFctType)(Can_HwHandleType Hrh,
                                         Can_IdType CanId,
                                         uint8_t CanDlc,
                                         const uint8_t* CanSduPtr);

/**
 * @brief TX confirmation callback type
 */
typedef void (*Can_TxConfirmationFctType)(PduIdType CanTxPduId);

/**
 * @brief Controller bus-off callback type
 */
typedef void (*Can_ControllerBusOffFctType)(uint8_t ControllerId);

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize CAN driver
 * @param[in] Config Pointer to configuration
 */
void Can_Init(const Can_ConfigType* Config);

/**
 * @brief De-initialize CAN driver
 */
void Can_DeInit(void);

/**
 * @brief Set CAN controller mode
 * @param[in] Controller Controller ID
 * @param[in] Transition Requested mode
 * @return CAN_OK on success
 */
Can_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ModeType Transition);

/**
 * @brief Get CAN controller error state
 * @param[in] ControllerId Controller ID
 * @param[out] ErrorStatePtr Pointer to error state
 * @return E_OK on success
 */
Std_ReturnType Can_GetControllerErrorState(uint8_t ControllerId,
                                            Can_ErrorStateType* ErrorStatePtr);

/**
 * @brief Get CAN controller mode
 * @param[in] Controller Controller ID
 * @param[out] ControllerModePtr Pointer to mode
 * @return E_OK on success
 */
Std_ReturnType Can_GetControllerMode(uint8_t Controller,
                                      Can_ControllerStateType* ControllerModePtr);

/**
 * @brief Write CAN message
 * @param[in] Hth Hardware transmit handle
 * @param[in] PduInfo Pointer to PDU info
 * @return CAN_OK on success
 */
Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo);

/**
 * @brief Main function for TX processing
 */
void Can_MainFunction_Write(void);

/**
 * @brief Main function for RX processing
 */
void Can_MainFunction_Read(void);

/**
 * @brief Main function for bus-off processing
 */
void Can_MainFunction_BusOff(void);

/**
 * @brief Main function for wakeup processing
 */
void Can_MainFunction_Wakeup(void);

/**
 * @brief Main function for mode transitions
 */
void Can_MainFunction_Mode(void);

/**
 * @brief Get version information
 * @param[out] versioninfo Pointer to version info
 */
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo);

/**
 * @brief Enable controller interrupts
 * @param[in] Controller Controller ID
 */
void Can_EnableControllerInterrupts(uint8_t Controller);

/**
 * @brief Disable controller interrupts
 * @param[in] Controller Controller ID
 */
void Can_DisableControllerInterrupts(uint8_t Controller);

/**
 * @brief Check wakeup
 * @param[in] Controller Controller ID
 * @return E_OK if wakeup detected
 */
Std_ReturnType Can_CheckWakeup(uint8_t Controller);

/*============================================================================*
 * CALLBACK CONFIGURATION
 *============================================================================*/

/**
 * @brief Set RX indication callback
 * @param[in] callback Callback function pointer
 */
void Can_SetRxIndicationCallback(Can_RxIndicationFctType callback);

/**
 * @brief Set TX confirmation callback
 * @param[in] callback Callback function pointer
 */
void Can_SetTxConfirmationCallback(Can_TxConfirmationFctType callback);

/**
 * @brief Set bus-off callback
 * @param[in] callback Callback function pointer
 */
void Can_SetControllerBusOffCallback(Can_ControllerBusOffFctType callback);

/*============================================================================*
 * SIMULATION FUNCTIONS (FOR TESTING)
 *============================================================================*/

/**
 * @brief Simulate RX message
 * @param[in] Controller Controller ID
 * @param[in] CanId CAN identifier
 * @param[in] Dlc Data length
 * @param[in] Data Pointer to data
 */
void Can_SimReceiveMessage(uint8_t Controller,
                           Can_IdType CanId,
                           uint8_t Dlc,
                           const uint8_t* Data);

/**
 * @brief Get last transmitted message
 * @param[out] CanId Pointer to CAN ID
 * @param[out] Dlc Pointer to DLC
 * @param[out] Data Pointer to data buffer (min 8 bytes)
 * @return TRUE if message available
 */
boolean Can_SimGetLastTxMessage(Can_IdType* CanId, uint8_t* Dlc, uint8_t* Data);

/**
 * @brief Clear RX buffer
 * @param[in] Controller Controller ID
 */
void Can_SimClearRxBuffer(uint8_t Controller);

/**
 * @brief Simulate bus-off condition
 * @param[in] Controller Controller ID
 */
void Can_SimTriggerBusOff(uint8_t Controller);

/**
 * @brief Get TX message count
 * @return Number of messages transmitted
 */
uint32_t Can_SimGetTxCount(void);

#endif /* CAN_H */
