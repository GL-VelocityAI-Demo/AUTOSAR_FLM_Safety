/**
 * @file Rte_Type.h
 * @brief RTE Type Definitions for FLM Application
 * @details Contains all RTE type definitions used by SWCs
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef RTE_TYPE_H
#define RTE_TYPE_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "ComStack_Types.h"

/*============================================================================*
 * RTE VERSION INFORMATION
 *============================================================================*/
#define RTE_TYPE_AR_RELEASE_MAJOR_VERSION    23
#define RTE_TYPE_AR_RELEASE_MINOR_VERSION    11

/*============================================================================*
 * FLM STATE MACHINE TYPES
 *============================================================================*/

/**
 * @brief FLM Application State Type
 * @details [FunSafReq01-03] State machine for safe state transition
 */
typedef enum {
    FLM_STATE_INIT      = 0x00U,    /**< Initialization state */
    FLM_STATE_NORMAL    = 0x01U,    /**< Normal operation */
    FLM_STATE_DEGRADED  = 0x02U,    /**< Degraded operation - one input invalid */
    FLM_STATE_SAFE      = 0x03U     /**< Safe state - critical fault */
} FLM_StateType;

/*============================================================================*
 * E2E TYPES
 *============================================================================*/

/**
 * @brief E2E Profile 01 Check Status
 * @details [SysSafReq02] E2E protection status
 */
typedef enum {
    E2E_P01STATUS_OK            = 0x00U,    /**< Check passed */
    E2E_P01STATUS_NONEWDATA     = 0x01U,    /**< No new data received */
    E2E_P01STATUS_WRONGCRC      = 0x02U,    /**< CRC check failed */
    E2E_P01STATUS_SYNC          = 0x03U,    /**< Synchronizing */
    E2E_P01STATUS_INITIAL       = 0x04U,    /**< Initial state */
    E2E_P01STATUS_REPEATED      = 0x05U,    /**< Repeated data */
    E2E_P01STATUS_OKSOMELOST    = 0x06U,    /**< OK but some data lost */
    E2E_P01STATUS_WRONGSEQUENCE = 0x07U     /**< Wrong sequence */
} E2E_P01CheckStatusType;

/**
 * @brief E2E State Machine Status
 */
typedef enum {
    E2E_SM_VALID        = 0x00U,    /**< Communication is valid */
    E2E_SM_DEINIT       = 0x01U,    /**< Not initialized */
    E2E_SM_NODATA       = 0x02U,    /**< No data available */
    E2E_SM_INIT         = 0x03U,    /**< Initialization */
    E2E_SM_INVALID      = 0x04U     /**< Communication is invalid */
} E2E_SMStateType;

/*============================================================================*
 * WATCHDOG MANAGER TYPES
 *============================================================================*/

/**
 * @brief Watchdog Manager Global Status
 * @details [SysSafReq03] Watchdog supervision status
 */
typedef enum {
    WDGM_GLOBAL_STATUS_OK               = 0x00U,    /**< All supervised entities OK */
    WDGM_GLOBAL_STATUS_FAILED           = 0x01U,    /**< At least one SE failed */
    WDGM_GLOBAL_STATUS_EXPIRED          = 0x02U,    /**< Watchdog expired */
    WDGM_GLOBAL_STATUS_STOPPED          = 0x03U,    /**< WdgM stopped */
    WDGM_GLOBAL_STATUS_DEACTIVATED      = 0x04U     /**< WdgM deactivated */
} WdgM_GlobalStatusType;

/**
 * @brief Watchdog Manager Local Status
 */
typedef enum {
    WDGM_LOCAL_STATUS_OK        = 0x00U,    /**< Supervised entity OK */
    WDGM_LOCAL_STATUS_FAILED    = 0x01U,    /**< Supervised entity failed */
    WDGM_LOCAL_STATUS_EXPIRED   = 0x02U,    /**< Supervision expired */
    WDGM_LOCAL_STATUS_DEACTIVATED = 0x03U   /**< Supervision deactivated */
} WdgM_LocalStatusType;

/**
 * @brief Supervision Entity ID Type
 */
typedef uint16_t WdgM_SupervisedEntityIdType;

/**
 * @brief Checkpoint ID Type
 */
typedef uint16_t WdgM_CheckpointIdType;

/*============================================================================*
 * DEM TYPES
 *============================================================================*/

/**
 * @brief DEM Event Status Type
 */
typedef enum {
    DEM_EVENT_STATUS_PASSED     = 0x00U,    /**< Event test passed */
    DEM_EVENT_STATUS_FAILED     = 0x01U,    /**< Event test failed */
    DEM_EVENT_STATUS_PREPASSED  = 0x02U,    /**< Pre-passed */
    DEM_EVENT_STATUS_PREFAILED  = 0x03U     /**< Pre-failed */
} Dem_EventStatusType;

/**
 * @brief DEM UDS Status Byte Type
 */
typedef uint8_t Dem_UdsStatusByteType;

/* UDS Status Byte Bits */
#define DEM_UDS_STATUS_TF       0x01U   /**< TestFailed */
#define DEM_UDS_STATUS_TFTOC    0x02U   /**< TestFailedThisOperationCycle */
#define DEM_UDS_STATUS_PDTC     0x04U   /**< PendingDTC */
#define DEM_UDS_STATUS_CDTC     0x08U   /**< ConfirmedDTC */
#define DEM_UDS_STATUS_TNCSLC   0x10U   /**< TestNotCompletedSinceLastClear */
#define DEM_UDS_STATUS_TFSLC    0x20U   /**< TestFailedSinceLastClear */
#define DEM_UDS_STATUS_TNCTOC   0x40U   /**< TestNotCompletedThisOperationCycle */
#define DEM_UDS_STATUS_WIR      0x80U   /**< WarningIndicatorRequested */

/*============================================================================*
 * APPLICATION DATA TYPES
 *============================================================================*/

/**
 * @brief Ambient light sensor raw data type
 */
typedef uint16_t Rte_AmbientLightRaw;

/**
 * @brief Headlight feedback current type (mA)
 */
typedef uint16_t Rte_HeadlightCurrent;

/**
 * @brief Timestamp type (milliseconds)
 */
typedef uint32_t Rte_TimestampType;

/**
 * @brief CAN message data buffer
 */
typedef struct {
    uint8_t data[8];            /**< CAN data (max 8 bytes) */
    uint8_t length;             /**< Actual data length */
    uint32_t timestamp;         /**< Reception timestamp */
    boolean isNew;              /**< New data flag */
} Rte_CanMessageType;

/*============================================================================*
 * RTE RETURN TYPES
 *============================================================================*/

/**
 * @brief RTE Standard Return Type
 */
typedef uint8_t Rte_StatusType;

#define RTE_E_OK                    0x00U   /**< No error */
#define RTE_E_INVALID               0x01U   /**< Invalid data */
#define RTE_E_COMMS_ERROR           0x02U   /**< Communication error */
#define RTE_E_TIMEOUT               0x03U   /**< Timeout occurred */
#define RTE_E_LIMIT                 0x04U   /**< Value out of limits */
#define RTE_E_NO_DATA               0x05U   /**< No data available */
#define RTE_E_TRANSMIT_ACK          0x06U   /**< Transmission acknowledged */
#define RTE_E_NEVER_RECEIVED        0x07U   /**< Never received */
#define RTE_E_UNCONNECTED           0x08U   /**< Port unconnected */
#define RTE_E_IN_EXCLUSIVE_AREA     0x09U   /**< In exclusive area */
#define RTE_E_SEG_FAULT             0x0AU   /**< Segmentation fault */
#define RTE_E_LOST_DATA             0x0BU   /**< Data lost */
#define RTE_E_MAX_AGE_EXCEEDED      0x0CU   /**< Max age exceeded */

/*============================================================================*
 * RTE INTERNAL DATA STRUCTURES
 *============================================================================*/

/**
 * @brief Inter-runnable variable for light switch data
 */
typedef struct {
    LightSwitchStatus status;
    Rte_TimestampType timestamp;
    E2E_P01CheckStatusType e2eStatus;
} Rte_IrvLightSwitchType;

/**
 * @brief Inter-runnable variable for ambient light data
 */
typedef struct {
    AmbientLightLevel level;
    SignalStatus status;
    Rte_TimestampType timestamp;
} Rte_IrvAmbientLightType;

/**
 * @brief Inter-runnable variable for headlight command
 */
typedef struct {
    HeadlightCommand command;
    FLM_StateType flmState;
    Rte_TimestampType timestamp;
} Rte_IrvHeadlightCmdType;

#endif /* RTE_TYPE_H */
