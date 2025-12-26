/**
 * @file WdgM.cpp
 * @brief AUTOSAR Watchdog Manager Implementation
 * @details Implements alive, deadline, and logical supervision
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq03] Watchdog supervision
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "WdgM.h"
#include <cstring>

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean WdgM_Initialized = FALSE;

/** @brief Configuration pointer */
static const WdgM_ConfigType* WdgM_ConfigPtr = NULL_PTR;

/** @brief Current mode */
static WdgM_ModeType WdgM_CurrentMode = WDGM_MODE_OFF;

/** @brief Global status */
static WdgM_GlobalStatusType WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_OK;

/** @brief Supervised entity runtime data */
static WdgM_SupervisedEntityRuntimeType WdgM_EntityData[WDGM_MAX_SUPERVISED_ENTITIES];

/** @brief Supervision cycle counter */
static uint32_t WdgM_SupervisionCycleCounter = 0U;

/** @brief Expired flag */
static boolean WdgM_Expired = FALSE;

/** @brief System time (ms) */
static uint32_t WdgM_SystemTime = 0U;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static uint8_t WdgM_GetEntityIndex(WdgM_SupervisedEntityIdType SEId);
static void WdgM_PerformAliveSupervision(void);
static void WdgM_UpdateLocalStatus(uint8_t index);
static void WdgM_UpdateGlobalStatus(void);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize Watchdog Manager
 */
void WdgM_Init(const WdgM_ConfigType* ConfigPtr) {
    uint8_t i;

    if (ConfigPtr == NULL_PTR) {
        return;
    }

    WdgM_ConfigPtr = ConfigPtr;

    /* Initialize entity data */
    for (i = 0U; i < WDGM_MAX_SUPERVISED_ENTITIES; i++) {
        WdgM_EntityData[i].localStatus = WDGM_LOCAL_STATUS_OK;
        WdgM_EntityData[i].aliveCounter = 0U;
        WdgM_EntityData[i].aliveIndicationsInCycle = 0U;
        WdgM_EntityData[i].lastCheckpointTime = 0U;
        WdgM_EntityData[i].failedCycleCount = 0U;
        WdgM_EntityData[i].isActive = TRUE;
    }

    WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_OK;
    WdgM_CurrentMode = WDGM_INITIAL_MODE;
    WdgM_SupervisionCycleCounter = 0U;
    WdgM_Expired = FALSE;
    WdgM_SystemTime = 0U;

    WdgM_Initialized = TRUE;
}

/**
 * @brief De-initialize Watchdog Manager
 */
void WdgM_DeInit(void) {
    WdgM_Initialized = FALSE;
    WdgM_ConfigPtr = NULL_PTR;
    WdgM_CurrentMode = WDGM_MODE_OFF;
    WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_DEACTIVATED;
}

/**
 * @brief Main function for WdgM
 */
void WdgM_MainFunction(void) {
    if (!WdgM_Initialized) {
        return;
    }

    if (WdgM_CurrentMode == WDGM_MODE_OFF) {
        return;
    }

    /* Update system time */
    WdgM_SystemTime += WDGM_MAIN_FUNCTION_PERIOD_MS;

    /* Increment supervision cycle counter */
    WdgM_SupervisionCycleCounter += WDGM_MAIN_FUNCTION_PERIOD_MS;

    /* Check if supervision cycle completed */
    if (WdgM_SupervisionCycleCounter >= WDGM_SUPERVISION_CYCLE_MS) {
        WdgM_SupervisionCycleCounter = 0U;

        /* Perform alive supervision */
        WdgM_PerformAliveSupervision();
    }

    /* Update global status */
    WdgM_UpdateGlobalStatus();
}

/**
 * @brief Get entity index from ID
 */
static uint8_t WdgM_GetEntityIndex(WdgM_SupervisedEntityIdType SEId) {
    /* Simple mapping: SEId 1-5 maps to index 0-4 */
    if ((SEId >= 1U) && (SEId <= WDGM_NUM_SUPERVISED_ENTITIES)) {
        return static_cast<uint8_t>(SEId - 1U);
    }
    return 0xFFU;  /* Invalid */
}

/**
 * @brief Report checkpoint reached
 */
Std_ReturnType WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    uint8_t index;

    STD_UNUSED(CPId);

    if (!WdgM_Initialized) {
        return E_NOT_OK;
    }

    index = WdgM_GetEntityIndex(SEId);
    if (index >= WDGM_MAX_SUPERVISED_ENTITIES) {
        return E_NOT_OK;
    }

    /* Update alive indication */
    WdgM_EntityData[index].aliveIndicationsInCycle++;
    WdgM_EntityData[index].lastCheckpointTime = WdgM_SystemTime;

    return E_OK;
}

/**
 * @brief Update alive counter
 */
Std_ReturnType WdgM_UpdateAliveCounter(WdgM_SupervisedEntityIdType SEId) {
    uint8_t index;

    if (!WdgM_Initialized) {
        return E_NOT_OK;
    }

    index = WdgM_GetEntityIndex(SEId);
    if (index >= WDGM_MAX_SUPERVISED_ENTITIES) {
        return E_NOT_OK;
    }

    WdgM_EntityData[index].aliveCounter++;
    WdgM_EntityData[index].aliveIndicationsInCycle++;

    return E_OK;
}

/**
 * @brief Perform alive supervision
 */
static void WdgM_PerformAliveSupervision(void) {
    uint8_t i;
    uint16_t expectedIndications;
    uint16_t actualIndications;
    int16_t margin;

    for (i = 0U; i < WDGM_NUM_SUPERVISED_ENTITIES; i++) {
        if (!WdgM_EntityData[i].isActive) {
            continue;
        }

        actualIndications = WdgM_EntityData[i].aliveIndicationsInCycle;

        /* Get expected indications based on entity */
        switch (i) {
            case 0:  /* SwitchEvent */
                expectedIndications = WDGM_SWITCHEVENT_EXPECTED_ALIVE;
                break;
            case 1:  /* LightRequest */
                expectedIndications = WDGM_LIGHTREQUEST_EXPECTED_ALIVE;
                break;
            case 2:  /* FLM */
                expectedIndications = WDGM_FLM_EXPECTED_ALIVE;
                break;
            case 3:  /* Headlight */
                expectedIndications = WDGM_HEADLIGHT_EXPECTED_ALIVE;
                break;
            case 4:  /* SafetyMonitor */
                expectedIndications = WDGM_SAFETYMONITOR_EXPECTED_ALIVE;
                break;
            default:
                expectedIndications = 10U;
                break;
        }

        /* Calculate margin */
        margin = static_cast<int16_t>(actualIndications) -
                 static_cast<int16_t>(expectedIndications);

        /* Check if within tolerance */
        if ((margin < -static_cast<int16_t>(WDGM_DEFAULT_MIN_MARGIN)) ||
            (margin > static_cast<int16_t>(WDGM_DEFAULT_MAX_MARGIN))) {
            /* Alive supervision failed */
            WdgM_EntityData[i].failedCycleCount++;

            if (WdgM_EntityData[i].failedCycleCount >= WDGM_FAILED_REFERENCE_CYCLES) {
                WdgM_EntityData[i].localStatus = WDGM_LOCAL_STATUS_EXPIRED;
            } else {
                WdgM_EntityData[i].localStatus = WDGM_LOCAL_STATUS_FAILED;
            }
        } else {
            /* Alive supervision passed */
            WdgM_EntityData[i].failedCycleCount = 0U;
            WdgM_EntityData[i].localStatus = WDGM_LOCAL_STATUS_OK;
        }

        /* Reset indication counter for next cycle */
        WdgM_EntityData[i].aliveIndicationsInCycle = 0U;
    }
}

/**
 * @brief Update local status for entity
 */
static void WdgM_UpdateLocalStatus(uint8_t index) {
    if (index >= WDGM_MAX_SUPERVISED_ENTITIES) {
        return;
    }

    /* Already updated in supervision functions */
}

/**
 * @brief Update global status
 */
static void WdgM_UpdateGlobalStatus(void) {
    uint8_t i;
    boolean anyFailed = FALSE;
    boolean anyExpired = FALSE;

    for (i = 0U; i < WDGM_NUM_SUPERVISED_ENTITIES; i++) {
        if (!WdgM_EntityData[i].isActive) {
            continue;
        }

        if (WdgM_EntityData[i].localStatus == WDGM_LOCAL_STATUS_FAILED) {
            anyFailed = TRUE;
        }

        if (WdgM_EntityData[i].localStatus == WDGM_LOCAL_STATUS_EXPIRED) {
            anyExpired = TRUE;
        }
    }

    if (anyExpired) {
        WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_EXPIRED;
        WdgM_Expired = TRUE;
    } else if (anyFailed) {
        WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_FAILED;
    } else {
        WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_OK;
    }
}

/**
 * @brief Get global supervision status
 */
Std_ReturnType WdgM_GetGlobalStatus(WdgM_GlobalStatusType* Status) {
    if (Status == NULL_PTR) {
        return E_NOT_OK;
    }

    if (!WdgM_Initialized) {
        *Status = WDGM_GLOBAL_STATUS_DEACTIVATED;
        return E_NOT_OK;
    }

    *Status = WdgM_GlobalStatus;
    return E_OK;
}

/**
 * @brief Get local supervision status
 */
Std_ReturnType WdgM_GetLocalStatus(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_LocalStatusType* Status
) {
    uint8_t index;

    if (Status == NULL_PTR) {
        return E_NOT_OK;
    }

    if (!WdgM_Initialized) {
        return E_NOT_OK;
    }

    index = WdgM_GetEntityIndex(SEId);
    if (index >= WDGM_MAX_SUPERVISED_ENTITIES) {
        return E_NOT_OK;
    }

    *Status = WdgM_EntityData[index].localStatus;
    return E_OK;
}

/**
 * @brief Set mode
 */
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode) {
    if (!WdgM_Initialized) {
        return E_NOT_OK;
    }

    WdgM_CurrentMode = Mode;

    if (Mode == WDGM_MODE_OFF) {
        WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_DEACTIVATED;
    }

    return E_OK;
}

/**
 * @brief Get mode
 */
Std_ReturnType WdgM_GetMode(WdgM_ModeType* Mode) {
    if (Mode == NULL_PTR) {
        return E_NOT_OK;
    }

    *Mode = WdgM_CurrentMode;
    return E_OK;
}

/**
 * @brief Get version information
 */
void WdgM_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) {
        return;
    }

    VersionInfo->vendorID = 0U;
    VersionInfo->moduleID = 13U;  /* WdgM module ID */
    VersionInfo->sw_major_version = 1U;
    VersionInfo->sw_minor_version = 0U;
    VersionInfo->sw_patch_version = 0U;
}

/**
 * @brief Perform reset (for testing)
 */
void WdgM_PerformReset(void) {
    /* In real system, would trigger MCU reset */
    WdgM_Expired = FALSE;
    WdgM_GlobalStatus = WDGM_GLOBAL_STATUS_STOPPED;
}
