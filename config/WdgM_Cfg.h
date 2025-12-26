/**
 * @file WdgM_Cfg.h
 * @brief Watchdog Manager Configuration
 * @details Configuration for AUTOSAR Watchdog Manager module
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq03] Watchdog supervision
 */

#ifndef WDGM_CFG_H
#define WDGM_CFG_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "FLM_Config.h"
#include "Rte/Rte_Type.h"

/*============================================================================*
 * WDGM GENERAL CONFIGURATION
 *============================================================================*/

/** @brief Enable development error detection */
#define WDGM_DEV_ERROR_DETECT               STD_ON

/** @brief Enable version info API */
#define WDGM_VERSION_INFO_API               STD_ON

/** @brief Enable defensive behavior */
#define WDGM_DEFENSIVE_BEHAVIOR             STD_ON

/** @brief Enable immediate reset on failure */
#define WDGM_IMMEDIATE_RESET                STD_OFF

/** @brief WdgM main function period (ms) */
#define WDGM_MAIN_FUNCTION_PERIOD_MS        FLM_WDGM_PERIOD_MS

/** @brief Supervision reference cycle (ms) */
#define WDGM_SUPERVISION_CYCLE_MS           FLM_WDGM_SUPERVISION_CYCLE_MS

/*============================================================================*
 * SUPERVISED ENTITY CONFIGURATION
 *============================================================================*/

/** @brief Maximum number of supervised entities */
#define WDGM_MAX_SUPERVISED_ENTITIES        8U

/** @brief Number of configured supervised entities */
#define WDGM_NUM_SUPERVISED_ENTITIES        5U

/*----------------------------------------------------------------------------*
 * Supervised Entity IDs (matching RTE definitions)
 *----------------------------------------------------------------------------*/

/** @brief SwitchEvent supervised entity ID */
#define WDGM_SE_SWITCHEVENT                 0x0001U

/** @brief LightRequest supervised entity ID */
#define WDGM_SE_LIGHTREQUEST                0x0002U

/** @brief FLM Application supervised entity ID */
#define WDGM_SE_FLM                         0x0003U

/** @brief Headlight supervised entity ID */
#define WDGM_SE_HEADLIGHT                   0x0004U

/** @brief SafetyMonitor supervised entity ID */
#define WDGM_SE_SAFETYMONITOR               0x0005U

/*============================================================================*
 * CHECKPOINT CONFIGURATION
 *============================================================================*/

/** @brief Maximum checkpoints per supervised entity */
#define WDGM_MAX_CHECKPOINTS_PER_SE         4U

/** @brief Total number of checkpoints */
#define WDGM_TOTAL_CHECKPOINTS              15U

/*----------------------------------------------------------------------------*
 * Checkpoint IDs for each supervised entity
 *----------------------------------------------------------------------------*/

/* SwitchEvent checkpoints */
#define WDGM_CP_SWITCHEVENT_ENTRY           0x0001U
#define WDGM_CP_SWITCHEVENT_EXIT            0x0002U

/* LightRequest checkpoints */
#define WDGM_CP_LIGHTREQUEST_ENTRY          0x0001U
#define WDGM_CP_LIGHTREQUEST_EXIT           0x0002U

/* FLM checkpoints */
#define WDGM_CP_FLM_ENTRY                   0x0001U
#define WDGM_CP_FLM_STATEMACHINE            0x0002U
#define WDGM_CP_FLM_EXIT                    0x0003U

/* Headlight checkpoints */
#define WDGM_CP_HEADLIGHT_ENTRY             0x0001U
#define WDGM_CP_HEADLIGHT_EXIT              0x0002U

/* SafetyMonitor checkpoints */
#define WDGM_CP_SAFETYMONITOR_ENTRY         0x0001U
#define WDGM_CP_SAFETYMONITOR_AGGREGATION   0x0002U
#define WDGM_CP_SAFETYMONITOR_EXIT          0x0003U

/*============================================================================*
 * ALIVE SUPERVISION CONFIGURATION
 *============================================================================*/

/**
 * @brief Alive supervision configuration type
 */
typedef struct {
    WdgM_SupervisedEntityIdType SEId;   /**< Supervised entity ID */
    uint16_t ExpectedAliveIndications;  /**< Expected alive indications per cycle */
    uint16_t MinMargin;                 /**< Minimum margin (tolerance) */
    uint16_t MaxMargin;                 /**< Maximum margin (tolerance) */
    uint16_t SupervisionReferenceCycle; /**< Reference cycle count */
} WdgM_AliveSupervisionConfigType;

/*----------------------------------------------------------------------------*
 * Alive Supervision Parameters
 *----------------------------------------------------------------------------*/

/** @brief SwitchEvent expected alive indications per 100ms */
#define WDGM_SWITCHEVENT_EXPECTED_ALIVE     10U  /* 10ms period */

/** @brief LightRequest expected alive indications per 100ms */
#define WDGM_LIGHTREQUEST_EXPECTED_ALIVE    5U   /* 20ms period */

/** @brief FLM expected alive indications per 100ms */
#define WDGM_FLM_EXPECTED_ALIVE             10U  /* 10ms period */

/** @brief Headlight expected alive indications per 100ms */
#define WDGM_HEADLIGHT_EXPECTED_ALIVE       10U  /* 10ms period */

/** @brief SafetyMonitor expected alive indications per 100ms */
#define WDGM_SAFETYMONITOR_EXPECTED_ALIVE   20U  /* 5ms period */

/** @brief Default minimum margin */
#define WDGM_DEFAULT_MIN_MARGIN             2U

/** @brief Default maximum margin */
#define WDGM_DEFAULT_MAX_MARGIN             2U

/*============================================================================*
 * DEADLINE SUPERVISION CONFIGURATION
 *============================================================================*/

/**
 * @brief Deadline supervision configuration type
 */
typedef struct {
    WdgM_SupervisedEntityIdType SEId;   /**< Supervised entity ID */
    WdgM_CheckpointIdType StartCP;      /**< Start checkpoint */
    WdgM_CheckpointIdType StopCP;       /**< Stop checkpoint */
    uint32_t DeadlineMin_us;            /**< Minimum deadline (microseconds) */
    uint32_t DeadlineMax_us;            /**< Maximum deadline (microseconds) */
} WdgM_DeadlineSupervisionConfigType;

/*----------------------------------------------------------------------------*
 * Deadline Supervision Parameters
 *----------------------------------------------------------------------------*/

/** @brief FLM main function maximum deadline (microseconds) */
#define WDGM_FLM_DEADLINE_MAX_US            5000U   /* 5ms max execution */

/** @brief Headlight main function maximum deadline (microseconds) */
#define WDGM_HEADLIGHT_DEADLINE_MAX_US      3000U   /* 3ms max execution */

/** @brief SafetyMonitor main function maximum deadline (microseconds) */
#define WDGM_SAFETYMONITOR_DEADLINE_MAX_US  2000U   /* 2ms max execution */

/*============================================================================*
 * LOGICAL SUPERVISION CONFIGURATION
 *============================================================================*/

/**
 * @brief Logical supervision transition type
 */
typedef struct {
    WdgM_CheckpointIdType SourceCP;     /**< Source checkpoint */
    WdgM_CheckpointIdType DestCP;       /**< Destination checkpoint */
} WdgM_LogicalTransitionType;

/**
 * @brief Logical supervision graph configuration type
 */
typedef struct {
    WdgM_SupervisedEntityIdType SEId;       /**< Supervised entity ID */
    WdgM_CheckpointIdType InitialCP;        /**< Initial checkpoint */
    WdgM_CheckpointIdType FinalCP;          /**< Final checkpoint */
    uint8_t NumTransitions;                 /**< Number of valid transitions */
    const WdgM_LogicalTransitionType* Transitions; /**< Valid transitions */
} WdgM_LogicalSupervisionConfigType;

/*============================================================================*
 * MODE CONFIGURATION
 *============================================================================*/

/**
 * @brief WdgM mode type
 */
typedef enum {
    WDGM_MODE_OFF     = 0U,     /**< Supervision off */
    WDGM_MODE_NORMAL  = 1U,     /**< Normal supervision */
    WDGM_MODE_REDUCED = 2U      /**< Reduced supervision */
} WdgM_ModeType;

/** @brief Initial mode after initialization */
#define WDGM_INITIAL_MODE                   WDGM_MODE_NORMAL

/*============================================================================*
 * FAILURE TOLERANCE CONFIGURATION
 *============================================================================*/

/** @brief Number of failed reference cycles before EXPIRED state */
#define WDGM_FAILED_REFERENCE_CYCLES        FLM_WDGM_FAILED_REF_CYCLE

/** @brief Enable global status callback */
#define WDGM_GLOBAL_STATUS_CALLBACK         STD_ON

/** @brief Enable local status callback */
#define WDGM_LOCAL_STATUS_CALLBACK          STD_ON

/*============================================================================*
 * SUPERVISED ENTITY CONFIGURATION STRUCTURE
 *============================================================================*/

/**
 * @brief Complete supervised entity configuration
 */
typedef struct {
    WdgM_SupervisedEntityIdType SEId;               /**< Entity ID */
    boolean AliveSupervisionEnabled;                /**< Alive supervision enabled */
    boolean DeadlineSupervisionEnabled;             /**< Deadline supervision enabled */
    boolean LogicalSupervisionEnabled;              /**< Logical supervision enabled */
    uint16_t FailedRefCycleCounter;                 /**< Failed cycles before expired */
    WdgM_LocalStatusType InitialStatus;             /**< Initial local status */
} WdgM_SupervisedEntityConfigType;

/*============================================================================*
 * CONFIGURATION DATA (EXTERN DECLARATIONS)
 *============================================================================*/

/** @brief Supervised entity configurations */
extern const WdgM_SupervisedEntityConfigType WdgM_SEConfig[WDGM_NUM_SUPERVISED_ENTITIES];

/** @brief Alive supervision configurations */
extern const WdgM_AliveSupervisionConfigType WdgM_AliveConfig[WDGM_NUM_SUPERVISED_ENTITIES];

/** @brief Deadline supervision configurations */
extern const WdgM_DeadlineSupervisionConfigType WdgM_DeadlineConfig[];

/** @brief Logical supervision configurations */
extern const WdgM_LogicalSupervisionConfigType WdgM_LogicalConfig[];

#endif /* WDGM_CFG_H */
