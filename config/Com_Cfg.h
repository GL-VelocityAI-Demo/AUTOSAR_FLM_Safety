/**
 * @file Com_Cfg.h
 * @brief COM Module Configuration
 * @details Configuration for AUTOSAR COM module
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef COM_CFG_H
#define COM_CFG_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "FLM_Config.h"

/*============================================================================*
 * COM GENERAL CONFIGURATION
 *============================================================================*/

/** @brief Enable development error detection */
#define COM_DEV_ERROR_DETECT                STD_ON

/** @brief Enable version info API */
#define COM_VERSION_INFO_API                STD_ON

/** @brief COM main function period (ms) */
#define COM_MAIN_FUNCTION_PERIOD_MS         10U

/** @brief Maximum number of I-PDUs */
#define COM_MAX_IPDU_COUNT                  16U

/** @brief Maximum number of signals */
#define COM_MAX_SIGNAL_COUNT                32U

/*============================================================================*
 * I-PDU CONFIGURATION
 *============================================================================*/

/** @brief I-PDU ID for light switch message (RX) */
#define COM_IPDU_LIGHTSWITCH_RX             0U

/** @brief I-PDU ID for light switch acknowledgement (TX) */
#define COM_IPDU_LIGHTSWITCH_ACK_TX         1U

/** @brief I-PDU ID for headlight status (TX) */
#define COM_IPDU_HEADLIGHT_STATUS_TX        2U

/** @brief Total number of I-PDUs configured */
#define COM_NUM_IPDUS                       3U

/*============================================================================*
 * SIGNAL CONFIGURATION
 *============================================================================*/

/** @brief Signal ID for light switch command */
#define COM_SIGNAL_LIGHTSWITCH_CMD          0U

/** @brief Signal ID for E2E counter */
#define COM_SIGNAL_E2E_COUNTER              1U

/** @brief Signal ID for E2E CRC */
#define COM_SIGNAL_E2E_CRC                  2U

/** @brief Signal ID for headlight state */
#define COM_SIGNAL_HEADLIGHT_STATE          3U

/** @brief Signal ID for fault status */
#define COM_SIGNAL_FAULT_STATUS             4U

/** @brief Total number of signals configured */
#define COM_NUM_SIGNALS                     5U

/*============================================================================*
 * SIGNAL GROUP CONFIGURATION
 *============================================================================*/

/** @brief Signal group ID for light switch message */
#define COM_SIGGROUP_LIGHTSWITCH            0U

/** @brief Total number of signal groups */
#define COM_NUM_SIGNAL_GROUPS               1U

/*============================================================================*
 * I-PDU PROPERTIES
 *============================================================================*/

/**
 * @brief I-PDU direction enumeration
 */
typedef enum {
    COM_IPDU_DIRECTION_RX = 0U,     /**< Receive I-PDU */
    COM_IPDU_DIRECTION_TX = 1U      /**< Transmit I-PDU */
} Com_IpduDirectionType;

/**
 * @brief I-PDU transmission mode
 */
typedef enum {
    COM_IPDU_MODE_PERIODIC = 0U,    /**< Periodic transmission */
    COM_IPDU_MODE_DIRECT   = 1U,    /**< Direct transmission */
    COM_IPDU_MODE_MIXED    = 2U     /**< Mixed mode */
} Com_IpduModeType;

/**
 * @brief Signal endianness
 */
typedef enum {
    COM_SIGNAL_LITTLE_ENDIAN = 0U,  /**< Little endian */
    COM_SIGNAL_BIG_ENDIAN    = 1U   /**< Big endian (Motorola) */
} Com_SignalEndiannessType;

/*============================================================================*
 * I-PDU CONFIGURATION STRUCTURE
 *============================================================================*/

/**
 * @brief I-PDU configuration type
 */
typedef struct {
    PduIdType IpduId;                   /**< I-PDU identifier */
    Com_IpduDirectionType Direction;    /**< Direction (RX/TX) */
    Com_IpduModeType Mode;              /**< Transmission mode */
    uint16_t Length;                    /**< I-PDU length in bytes */
    uint16_t Period;                    /**< Transmission period (ms) */
    uint16_t Timeout;                   /**< Reception timeout (ms) */
    boolean E2EProtected;               /**< E2E protection enabled */
} Com_IpduConfigType;

/**
 * @brief Signal configuration type
 */
typedef struct {
    uint16_t SignalId;                  /**< Signal identifier */
    PduIdType IpduId;                   /**< Parent I-PDU ID */
    uint16_t BitPosition;               /**< Bit position in I-PDU */
    uint16_t BitSize;                   /**< Signal size in bits */
    Com_SignalEndiannessType Endianness;/**< Signal endianness */
    uint32_t InitValue;                 /**< Initial value */
} Com_SignalConfigType;

/*============================================================================*
 * LIGHT SWITCH MESSAGE CONFIGURATION
 *============================================================================*/

/**
 * @brief Light switch message layout
 * Byte 0: CRC (8 bits)
 * Byte 1: Counter (4 bits low nibble) + Reserved (4 bits high nibble)
 * Byte 2: Light switch command (8 bits)
 * Byte 3: Reserved
 */

/** @brief CRC byte position */
#define COM_LIGHTSWITCH_CRC_BYTE            0U

/** @brief Counter byte position */
#define COM_LIGHTSWITCH_COUNTER_BYTE        1U

/** @brief Command byte position */
#define COM_LIGHTSWITCH_CMD_BYTE            2U

/** @brief Message length */
#define COM_LIGHTSWITCH_LENGTH              4U

/*============================================================================*
 * TIMEOUT CONFIGURATION
 *============================================================================*/

/** @brief First timeout notification (ms) */
#define COM_FIRST_TIMEOUT_MS                50U

/** @brief Deadline monitoring enabled */
#define COM_DEADLINE_MONITORING             STD_ON

/** @brief Reception timeout for light switch (ms) */
#define COM_LIGHTSWITCH_RX_TIMEOUT_MS       FLM_CAN_TIMEOUT_MS

/*============================================================================*
 * CALLBACK CONFIGURATION
 *============================================================================*/

/** @brief Enable RX indication callback */
#define COM_RX_INDICATION_ENABLED           STD_ON

/** @brief Enable TX confirmation callback */
#define COM_TX_CONFIRMATION_ENABLED         STD_ON

/** @brief Enable timeout notification */
#define COM_TIMEOUT_NOTIFICATION_ENABLED    STD_ON

/*============================================================================*
 * FILTER CONFIGURATION
 *============================================================================*/

/**
 * @brief Signal filter type
 */
typedef enum {
    COM_FILTER_ALWAYS    = 0U,      /**< Always pass */
    COM_FILTER_NEVER     = 1U,      /**< Never pass */
    COM_FILTER_NEW_VALUE = 2U,      /**< Pass on new value */
    COM_FILTER_MASKED    = 3U       /**< Pass based on mask */
} Com_FilterType;

/*============================================================================*
 * CONFIGURATION DATA (EXTERN DECLARATIONS)
 *============================================================================*/

/** @brief I-PDU configurations */
extern const Com_IpduConfigType Com_IpduConfig[COM_NUM_IPDUS];

/** @brief Signal configurations */
extern const Com_SignalConfigType Com_SignalConfig[COM_NUM_SIGNALS];

#endif /* COM_CFG_H */
