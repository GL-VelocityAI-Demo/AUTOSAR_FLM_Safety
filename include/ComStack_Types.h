/**
 * @file ComStack_Types.h
 * @brief AUTOSAR Communication Stack Types Definition
 * @details This file contains the AUTOSAR communication stack types as defined in
 *          AUTOSAR_SWS_CommunicationStackTypes specification.
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"

/*============================================================================*
 * AUTOSAR VERSION INFORMATION
 *============================================================================*/
#define COMSTACK_TYPES_AR_RELEASE_MAJOR_VERSION    23
#define COMSTACK_TYPES_AR_RELEASE_MINOR_VERSION    11
#define COMSTACK_TYPES_AR_RELEASE_PATCH_VERSION    0

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief PDU ID type
 * @details Used to identify a PDU (Protocol Data Unit)
 */
typedef uint16_t PduIdType;

/**
 * @brief PDU length type
 * @details Used to store the length of a PDU
 */
typedef uint32_t PduLengthType;

/**
 * @brief PDU info type
 * @details Contains pointer to SDU data and length information
 */
typedef struct {
    uint8_t* SduDataPtr;        /**< Pointer to SDU data */
    uint8_t* MetaDataPtr;       /**< Pointer to meta data */
    PduLengthType SduLength;    /**< Length of SDU in bytes */
} PduInfoType;

/**
 * @brief Notification result type
 * @details Used to notify upper layer about transmission result
 */
typedef enum {
    NTFRSLT_OK              = 0x00U,    /**< Transmission successful */
    NTFRSLT_E_NOT_OK        = 0x01U,    /**< Transmission failed */
    NTFRSLT_E_TIMEOUT_A     = 0x02U,    /**< Timeout A occurred */
    NTFRSLT_E_TIMEOUT_BS    = 0x03U,    /**< Timeout BS occurred */
    NTFRSLT_E_TIMEOUT_CR    = 0x04U,    /**< Timeout CR occurred */
    NTFRSLT_E_WRONG_SN      = 0x05U,    /**< Wrong sequence number */
    NTFRSLT_E_INVALID_FS    = 0x06U,    /**< Invalid flow status */
    NTFRSLT_E_UNEXP_PDU     = 0x07U,    /**< Unexpected PDU */
    NTFRSLT_E_WFT_OVRN      = 0x08U,    /**< WFT counter overrun */
    NTFRSLT_E_ABORT         = 0x09U,    /**< Transmission aborted */
    NTFRSLT_E_NO_BUFFER     = 0x0AU,    /**< No buffer available */
    NTFRSLT_E_CANCELATION_OK = 0x0BU,   /**< Cancellation successful */
    NTFRSLT_E_CANCELATION_NOT_OK = 0x0CU /**< Cancellation failed */
} NotifResultType;

/**
 * @brief Buffer request type
 * @details Indicates if buffer is available
 */
typedef enum {
    BUFREQ_OK       = 0x00U,    /**< Buffer request successful */
    BUFREQ_E_NOT_OK = 0x01U,    /**< Buffer request failed */
    BUFREQ_E_BUSY   = 0x02U,    /**< Buffer currently busy */
    BUFREQ_E_OVFL   = 0x03U     /**< Buffer overflow */
} BufReq_ReturnType;

/**
 * @brief Retry info type
 * @details For retry mechanism in transport protocol
 */
typedef enum {
    TP_DATACONF     = 0x00U,    /**< Transmission confirmed */
    TP_DATARETRY    = 0x01U,    /**< Retry required */
    TP_CONFPENDING  = 0x02U     /**< Confirmation pending */
} TpDataStateType;

/**
 * @brief Retry info structure
 */
typedef struct {
    TpDataStateType TpDataState;    /**< Current state */
    PduLengthType TxTpDataCnt;      /**< Number of transmitted bytes */
} RetryInfoType;

/**
 * @brief Network handle type
 * @details Identifies a communication channel
 */
typedef uint8_t NetworkHandleType;

/**
 * @brief Communication mode type
 * @details Defines the communication mode
 */
typedef enum {
    COMM_NO_COMMUNICATION       = 0x00U,    /**< No communication */
    COMM_SILENT_COMMUNICATION   = 0x01U,    /**< Silent communication (receive only) */
    COMM_FULL_COMMUNICATION     = 0x02U     /**< Full communication */
} ComM_ModeType;

/**
 * @brief Inhibition status type
 */
typedef uint8_t ComM_InhibitionStatusType;

/**
 * @brief User handle type
 */
typedef uint8_t ComM_UserHandleType;

/*============================================================================*
 * CAN SPECIFIC TYPES
 *============================================================================*/

/**
 * @brief CAN ID type
 * @details Standard or extended CAN identifier
 */
typedef uint32_t Can_IdType;

/**
 * @brief CAN hardware handle type
 */
typedef uint16_t Can_HwHandleType;

/**
 * @brief CAN PDU type
 */
typedef struct {
    PduIdType swPduHandle;      /**< PDU handle from COM */
    uint8_t length;             /**< Data length */
    Can_IdType id;              /**< CAN ID */
    uint8_t* sdu;               /**< Pointer to data */
} Can_PduType;

/**
 * @brief CAN return type
 */
typedef enum {
    CAN_OK      = 0x00U,        /**< Operation successful */
    CAN_NOT_OK  = 0x01U,        /**< Operation failed */
    CAN_BUSY    = 0x02U         /**< CAN busy */
} Can_ReturnType;

/**
 * @brief CAN state transition type
 */
typedef enum {
    CAN_CS_UNINIT   = 0x00U,    /**< Uninitialized */
    CAN_CS_STARTED  = 0x01U,    /**< Started */
    CAN_CS_STOPPED  = 0x02U,    /**< Stopped */
    CAN_CS_SLEEP    = 0x03U     /**< Sleep mode */
} Can_ControllerStateType;

/**
 * @brief CAN error state type
 */
typedef enum {
    CAN_ERRORSTATE_ACTIVE   = 0x00U,    /**< Error active */
    CAN_ERRORSTATE_PASSIVE  = 0x01U,    /**< Error passive */
    CAN_ERRORSTATE_BUSOFF   = 0x02U     /**< Bus off */
} Can_ErrorStateType;

/*============================================================================*
 * FLM SPECIFIC TYPES
 *============================================================================*/

/**
 * @brief Light switch command from CAN
 */
typedef enum {
    LIGHT_SWITCH_OFF        = 0x00U,    /**< Lights OFF */
    LIGHT_SWITCH_LOW_BEAM   = 0x01U,    /**< Low beam ON */
    LIGHT_SWITCH_HIGH_BEAM  = 0x02U,    /**< High beam ON */
    LIGHT_SWITCH_AUTO       = 0x03U     /**< Automatic mode */
} LightSwitchCmd;

/**
 * @brief Light switch status with validity
 */
typedef struct {
    LightSwitchCmd command;     /**< Switch command */
    boolean isValid;            /**< Validity flag */
    uint8_t e2eStatus;          /**< E2E protection status */
} LightSwitchStatus;

/**
 * @brief Ambient light level type
 */
typedef struct {
    uint16_t adcValue;          /**< Raw ADC value (0-4095) */
    uint16_t luxValue;          /**< Calculated lux value */
    boolean isValid;            /**< Validity flag */
} AmbientLightLevel;

/**
 * @brief Signal status enumeration
 */
typedef enum {
    SIGNAL_STATUS_VALID         = 0x00U,    /**< Signal is valid */
    SIGNAL_STATUS_INVALID       = 0x01U,    /**< Signal is invalid */
    SIGNAL_STATUS_TIMEOUT       = 0x02U,    /**< Signal timeout */
    SIGNAL_STATUS_OPEN_CIRCUIT  = 0x03U,    /**< Open circuit detected */
    SIGNAL_STATUS_SHORT_CIRCUIT = 0x04U,    /**< Short circuit detected */
    SIGNAL_STATUS_PLAUSIBILITY  = 0x05U     /**< Plausibility error */
} SignalStatus;

/**
 * @brief Headlight command type
 */
typedef enum {
    HEADLIGHT_CMD_OFF       = 0x00U,    /**< Headlight OFF */
    HEADLIGHT_CMD_LOW_BEAM  = 0x01U,    /**< Low beam ON */
    HEADLIGHT_CMD_HIGH_BEAM = 0x02U     /**< High beam ON */
} HeadlightCommand;

/**
 * @brief Headlight fault status
 */
typedef enum {
    HEADLIGHT_FAULT_NONE        = 0x00U,    /**< No fault */
    HEADLIGHT_FAULT_OPEN_LOAD   = 0x01U,    /**< Open load detected */
    HEADLIGHT_FAULT_SHORT       = 0x02U,    /**< Short circuit detected */
    HEADLIGHT_FAULT_OVERCURRENT = 0x03U     /**< Overcurrent detected */
} HeadlightFaultStatus;

/**
 * @brief Safety status type
 */
typedef enum {
    SAFETY_STATUS_OK            = 0x00U,    /**< All OK */
    SAFETY_STATUS_WARNING       = 0x01U,    /**< Warning level */
    SAFETY_STATUS_DEGRADED      = 0x02U,    /**< Degraded operation */
    SAFETY_STATUS_SAFE_STATE    = 0x03U     /**< Safe state active */
} SafetyStatusType;

/**
 * @brief Safe state reason
 */
typedef enum {
    SAFE_STATE_REASON_NONE          = 0x00U,    /**< No safe state */
    SAFE_STATE_REASON_E2E_FAILURE   = 0x01U,    /**< E2E protection failure */
    SAFE_STATE_REASON_WDGM_FAILURE  = 0x02U,    /**< Watchdog failure */
    SAFE_STATE_REASON_MULTI_FAULT   = 0x03U,    /**< Multiple faults */
    SAFE_STATE_REASON_TIMEOUT       = 0x04U,    /**< Timeout occurred */
    SAFE_STATE_REASON_MANUAL        = 0x05U     /**< Manual trigger */
} SafeStateReason;

#endif /* COMSTACK_TYPES_H */
