/**
 * @file Dem_Cfg.h
 * @brief Diagnostic Event Manager Configuration
 * @details Configuration for AUTOSAR DEM module
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef DEM_CFG_H
#define DEM_CFG_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "Rte/Rte_Type.h"

/*============================================================================*
 * DEM GENERAL CONFIGURATION
 *============================================================================*/

/** @brief Enable development error detection */
#define DEM_DEV_ERROR_DETECT                STD_ON

/** @brief Enable version info API */
#define DEM_VERSION_INFO_API                STD_ON

/** @brief DEM status byte storage */
#define DEM_STATUS_BYTE_STORAGE             STD_ON

/** @brief Enable aging */
#define DEM_AGING_ENABLED                   STD_ON

/** @brief Enable occurrence counter */
#define DEM_OCCURRENCE_COUNTER_ENABLED      STD_ON

/*============================================================================*
 * EVENT CONFIGURATION
 *============================================================================*/

/** @brief Maximum number of events */
#define DEM_MAX_NUM_EVENTS                  16U

/** @brief Number of configured events */
#define DEM_NUM_EVENTS                      10U

/** @brief Event memory size (number of stored events) */
#define DEM_EVENT_MEMORY_SIZE               8U

/*============================================================================*
 * DTC CONFIGURATION
 *============================================================================*/

/** @brief DTC format (3-byte UDS format) */
#define DEM_DTC_FORMAT_UDS                  0x01U

/** @brief DTC status availability mask */
#define DEM_DTC_STATUS_AVAILABILITY_MASK    0xFFU

/*============================================================================*
 * EVENT ID DEFINITIONS
 *============================================================================*/

/**
 * @brief DEM Event IDs
 * @details Event IDs for all diagnostic events in the FLM system
 */
typedef enum {
    /** @brief Invalid event (reserved) */
    DEM_EVENT_INVALID                       = 0x00U,

    /** @brief E2E light switch failed [SysSafReq02] */
    DEM_EVENT_E2E_LIGHTSWITCH_FAILED        = 0x01U,

    /** @brief Ambient light sensor open circuit [FunSafReq01-02] */
    DEM_EVENT_AMBIENTLIGHT_OPEN_CIRCUIT     = 0x02U,

    /** @brief Ambient light sensor short circuit [FunSafReq01-02] */
    DEM_EVENT_AMBIENTLIGHT_SHORT_CIRCUIT    = 0x03U,

    /** @brief Ambient light plausibility error [FunSafReq01-02] */
    DEM_EVENT_AMBIENTLIGHT_PLAUSIBILITY     = 0x04U,

    /** @brief Headlight open load [SysSafReq10] */
    DEM_EVENT_HEADLIGHT_OPEN_LOAD           = 0x05U,

    /** @brief Headlight short circuit [SysSafReq10] */
    DEM_EVENT_HEADLIGHT_SHORT_CIRCUIT       = 0x06U,

    /** @brief CAN timeout [SysSafReq01] */
    DEM_EVENT_CAN_TIMEOUT                   = 0x07U,

    /** @brief WdgM supervision failed [SysSafReq03] */
    DEM_EVENT_WDGM_SUPERVISION_FAILED       = 0x08U,

    /** @brief Safe state entered [FunSafReq01-03] */
    DEM_EVENT_SAFE_STATE_ENTERED            = 0x09U,

    /** @brief Maximum event ID */
    DEM_EVENT_MAX                           = 0x0AU
} DEM_EventIdType;

/*============================================================================*
 * DTC DEFINITIONS
 *============================================================================*/

/**
 * @brief DTC values (3-byte UDS format)
 */
#define DEM_DTC_E2E_FAILURE                 0xC10100U
#define DEM_DTC_AMBIENT_OPEN                0xC20100U
#define DEM_DTC_AMBIENT_SHORT               0xC20200U
#define DEM_DTC_AMBIENT_PLAUSIBILITY        0xC20300U
#define DEM_DTC_HEADLIGHT_OPEN              0xC30100U
#define DEM_DTC_HEADLIGHT_SHORT             0xC30200U
#define DEM_DTC_CAN_TIMEOUT                 0xC40100U
#define DEM_DTC_WDGM_FAILED                 0xC50100U
#define DEM_DTC_SAFE_STATE                  0xC60100U

/*============================================================================*
 * DEBOUNCE CONFIGURATION
 *============================================================================*/

/**
 * @brief Debounce algorithm type
 */
typedef enum {
    DEM_DEBOUNCE_COUNTER_BASED = 0U,    /**< Counter-based debouncing */
    DEM_DEBOUNCE_TIME_BASED    = 1U     /**< Time-based debouncing */
} Dem_DebounceAlgorithmType;

/** @brief Default debounce type */
#define DEM_DEFAULT_DEBOUNCE_ALGO           DEM_DEBOUNCE_COUNTER_BASED

/** @brief Pass counter threshold */
#define DEM_DEBOUNCE_PASS_THRESHOLD         (-3)

/** @brief Fail counter threshold */
#define DEM_DEBOUNCE_FAIL_THRESHOLD         3

/** @brief Jump up value */
#define DEM_DEBOUNCE_JUMP_UP                1

/** @brief Jump down value */
#define DEM_DEBOUNCE_JUMP_DOWN              (-1)

/*============================================================================*
 * EVENT CONFIGURATION STRUCTURE
 *============================================================================*/

/**
 * @brief Event configuration type
 */
typedef struct {
    DEM_EventIdType EventId;            /**< Event identifier */
    uint32_t DTCValue;                  /**< Associated DTC value */
    uint8_t EventKind;                  /**< Event kind (BSW/SWC) */
    Dem_DebounceAlgorithmType DebounceAlgo; /**< Debounce algorithm */
    sint16_t FailThreshold;             /**< Fail threshold */
    sint16_t PassThreshold;             /**< Pass threshold */
    uint16_t AgingThreshold;            /**< Aging cycles threshold */
    boolean EnableStorage;              /**< Enable event storage */
    boolean EnableAging;                /**< Enable aging */
} Dem_EventConfigType;

/**
 * @brief Event kind definitions
 */
#define DEM_EVENT_KIND_BSW                  0x00U   /**< BSW event */
#define DEM_EVENT_KIND_SWC                  0x01U   /**< SWC event */

/*============================================================================*
 * OPERATION CYCLE CONFIGURATION
 *============================================================================*/

/**
 * @brief Operation cycle ID type
 */
typedef uint8_t Dem_OperationCycleIdType;

/** @brief Ignition cycle */
#define DEM_OPCYCLE_IGNITION                0x00U

/** @brief Power cycle */
#define DEM_OPCYCLE_POWER                   0x01U

/** @brief Number of operation cycles */
#define DEM_NUM_OPERATION_CYCLES            2U

/*============================================================================*
 * AGING CONFIGURATION
 *============================================================================*/

/** @brief Default aging threshold (operation cycles) */
#define DEM_DEFAULT_AGING_THRESHOLD         40U

/** @brief Healing threshold for prefailed events */
#define DEM_HEALING_THRESHOLD               3U

/*============================================================================*
 * FREEZE FRAME CONFIGURATION
 *============================================================================*/

/** @brief Enable freeze frame storage */
#define DEM_FREEZE_FRAME_STORAGE            STD_ON

/** @brief Maximum freeze frames per event */
#define DEM_MAX_FREEZE_FRAMES_PER_EVENT     1U

/** @brief Freeze frame data size */
#define DEM_FREEZE_FRAME_SIZE               8U

/**
 * @brief Freeze frame data identifiers
 */
#define DEM_DID_AMBIENT_LIGHT               0xF100U
#define DEM_DID_HEADLIGHT_STATE             0xF101U
#define DEM_DID_E2E_STATUS                  0xF102U
#define DEM_DID_FLM_STATE                   0xF103U

/*============================================================================*
 * EXTENDED DATA CONFIGURATION
 *============================================================================*/

/** @brief Enable extended data storage */
#define DEM_EXTENDED_DATA_STORAGE           STD_ON

/** @brief Extended data record size */
#define DEM_EXTENDED_DATA_SIZE              4U

/*============================================================================*
 * ENABLE CONDITION CONFIGURATION
 *============================================================================*/

/** @brief Number of enable conditions */
#define DEM_NUM_ENABLE_CONDITIONS           2U

/** @brief Battery voltage OK condition */
#define DEM_ENABLE_COND_BATTERY_OK          0x00U

/** @brief Communication active condition */
#define DEM_ENABLE_COND_COMM_ACTIVE         0x01U

/*============================================================================*
 * CALLBACK CONFIGURATION
 *============================================================================*/

/** @brief Enable event status changed callback */
#define DEM_EVENT_STATUS_CHANGED_CALLBACK   STD_ON

/** @brief Enable DTC status changed callback */
#define DEM_DTC_STATUS_CHANGED_CALLBACK     STD_ON

/** @brief Enable clear DTC notification */
#define DEM_CLEAR_DTC_NOTIFICATION          STD_ON

/*============================================================================*
 * CONFIGURATION DATA (EXTERN DECLARATIONS)
 *============================================================================*/

/** @brief Event configurations */
extern const Dem_EventConfigType Dem_EventConfig[DEM_NUM_EVENTS];

#endif /* DEM_CFG_H */
