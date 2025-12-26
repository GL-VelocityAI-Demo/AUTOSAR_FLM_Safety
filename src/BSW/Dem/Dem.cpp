/**
 * @file Dem.cpp
 * @brief AUTOSAR Diagnostic Event Manager Implementation
 * @details DEM module for diagnostic event and DTC management
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Dem.h"
#include <cstring>

/*============================================================================*
 * LOCAL TYPES
 *============================================================================*/

/**
 * @brief Event runtime data
 */
typedef struct {
    Dem_UdsStatusByteType udsStatus;
    sint16_t debounceCounter;
    uint16_t occurrenceCounter;
    boolean stored;
} Dem_EventDataType;

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean Dem_Initialized = FALSE;

/** @brief DTC setting enabled */
static boolean Dem_DTCSettingEnabled = TRUE;

/** @brief Event runtime data */
static Dem_EventDataType Dem_EventData[DEM_MAX_NUM_EVENTS];

/** @brief Number of stored events */
static uint16_t Dem_StoredEventCount = 0U;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void Dem_ProcessDebounce(DEM_EventIdType EventId, Dem_EventStatusType Status);
static void Dem_UpdateUdsStatus(DEM_EventIdType EventId, boolean testFailed);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Pre-initialize DEM module
 */
void Dem_PreInit(void) {
    uint16_t i;

    /* Initialize event data */
    for (i = 0U; i < DEM_MAX_NUM_EVENTS; i++) {
        Dem_EventData[i].udsStatus = 0x50U;  /* TNCSLC | TNCTOC */
        Dem_EventData[i].debounceCounter = 0;
        Dem_EventData[i].occurrenceCounter = 0U;
        Dem_EventData[i].stored = FALSE;
    }

    Dem_StoredEventCount = 0U;
}

/**
 * @brief Initialize DEM module
 */
void Dem_Init(void) {
    Dem_PreInit();
    Dem_DTCSettingEnabled = TRUE;
    Dem_Initialized = TRUE;
}

/**
 * @brief Shutdown DEM module
 */
void Dem_Shutdown(void) {
    /* Store events to NvM would happen here */
    Dem_Initialized = FALSE;
}

/**
 * @brief Main function for DEM
 */
void Dem_MainFunction(void) {
    if (!Dem_Initialized) {
        return;
    }

    /* Aging and other periodic processing would happen here */
}

/**
 * @brief Set event status
 */
Std_ReturnType Dem_SetEventStatus(
    DEM_EventIdType EventId,
    Dem_EventStatusType EventStatus
) {
    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    if (EventId >= DEM_MAX_NUM_EVENTS) {
        return E_NOT_OK;
    }

    if (!Dem_DTCSettingEnabled) {
        return E_NOT_OK;
    }

    /* Process debounce */
    Dem_ProcessDebounce(EventId, EventStatus);

    return E_OK;
}

/**
 * @brief Process debounce algorithm
 */
static void Dem_ProcessDebounce(DEM_EventIdType EventId, Dem_EventStatusType Status) {
    sint16_t* counter;
    boolean testFailed = FALSE;

    if (EventId >= DEM_MAX_NUM_EVENTS) {
        return;
    }

    counter = &Dem_EventData[EventId].debounceCounter;

    switch (Status) {
        case DEM_EVENT_STATUS_PASSED:
            *counter = DEM_DEBOUNCE_PASS_THRESHOLD;
            testFailed = FALSE;
            break;

        case DEM_EVENT_STATUS_FAILED:
            *counter = DEM_DEBOUNCE_FAIL_THRESHOLD;
            testFailed = TRUE;
            break;

        case DEM_EVENT_STATUS_PREPASSED:
            *counter += DEM_DEBOUNCE_JUMP_DOWN;
            if (*counter <= DEM_DEBOUNCE_PASS_THRESHOLD) {
                *counter = DEM_DEBOUNCE_PASS_THRESHOLD;
                testFailed = FALSE;
            }
            break;

        case DEM_EVENT_STATUS_PREFAILED:
            *counter += DEM_DEBOUNCE_JUMP_UP;
            if (*counter >= DEM_DEBOUNCE_FAIL_THRESHOLD) {
                *counter = DEM_DEBOUNCE_FAIL_THRESHOLD;
                testFailed = TRUE;
            }
            break;

        default:
            break;
    }

    /* Update UDS status if threshold reached */
    if ((*counter <= DEM_DEBOUNCE_PASS_THRESHOLD) ||
        (*counter >= DEM_DEBOUNCE_FAIL_THRESHOLD)) {
        Dem_UpdateUdsStatus(EventId, testFailed);
    }
}

/**
 * @brief Update UDS status byte
 */
static void Dem_UpdateUdsStatus(DEM_EventIdType EventId, boolean testFailed) {
    Dem_UdsStatusByteType* status;

    if (EventId >= DEM_MAX_NUM_EVENTS) {
        return;
    }

    status = &Dem_EventData[EventId].udsStatus;

    /* Clear TNCTOC - test completed this operation cycle */
    *status &= ~DEM_UDS_STATUS_TNCTOC;

    if (testFailed) {
        /* Set TF - test failed */
        *status |= DEM_UDS_STATUS_TF;

        /* Set TFTOC - test failed this operation cycle */
        *status |= DEM_UDS_STATUS_TFTOC;

        /* Set TFSLC - test failed since last clear */
        *status |= DEM_UDS_STATUS_TFSLC;

        /* Clear TNCSLC - test completed since last clear */
        *status &= ~DEM_UDS_STATUS_TNCSLC;

        /* Set PDTC - pending DTC */
        *status |= DEM_UDS_STATUS_PDTC;

        /* Increment occurrence counter */
        if (Dem_EventData[EventId].occurrenceCounter < 0xFFFFU) {
            Dem_EventData[EventId].occurrenceCounter++;
        }

        /* Store event if not already stored */
        if (!Dem_EventData[EventId].stored) {
            if (Dem_StoredEventCount < DEM_EVENT_MEMORY_SIZE) {
                Dem_EventData[EventId].stored = TRUE;
                Dem_StoredEventCount++;

                /* Set CDTC - confirmed DTC */
                *status |= DEM_UDS_STATUS_CDTC;
            }
        }
    } else {
        /* Clear TF - test passed */
        *status &= ~DEM_UDS_STATUS_TF;

        /* Clear TFTOC */
        *status &= ~DEM_UDS_STATUS_TFTOC;
    }
}

/**
 * @brief Get event status
 */
Std_ReturnType Dem_GetEventStatus(
    DEM_EventIdType EventId,
    Dem_UdsStatusByteType* EventStatus
) {
    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    if ((EventId >= DEM_MAX_NUM_EVENTS) || (EventStatus == NULL_PTR)) {
        return E_NOT_OK;
    }

    *EventStatus = Dem_EventData[EventId].udsStatus;

    return E_OK;
}

/**
 * @brief Reset event status
 */
Std_ReturnType Dem_ResetEventStatus(DEM_EventIdType EventId) {
    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    if (EventId >= DEM_MAX_NUM_EVENTS) {
        return E_NOT_OK;
    }

    Dem_EventData[EventId].udsStatus = 0x50U;
    Dem_EventData[EventId].debounceCounter = 0;

    return E_OK;
}

/**
 * @brief Clear DTC
 */
Std_ReturnType Dem_ClearDTC(uint32_t DTC) {
    uint16_t i;

    STD_UNUSED(DTC);

    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    /* Clear all events (simplified - in real system would match DTC) */
    for (i = 0U; i < DEM_MAX_NUM_EVENTS; i++) {
        Dem_EventData[i].udsStatus = 0x50U;
        Dem_EventData[i].debounceCounter = 0;
        Dem_EventData[i].occurrenceCounter = 0U;
        Dem_EventData[i].stored = FALSE;
    }

    Dem_StoredEventCount = 0U;

    return E_OK;
}

/**
 * @brief Get DTC status
 */
Std_ReturnType Dem_GetDTCStatus(uint32_t DTC, Dem_UdsStatusByteType* Status) {
    STD_UNUSED(DTC);

    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    if (Status == NULL_PTR) {
        return E_NOT_OK;
    }

    /* Simplified - return first matching event status */
    *Status = 0x00U;

    return E_OK;
}

/**
 * @brief Report operation cycle state
 */
Std_ReturnType Dem_SetOperationCycleState(
    Dem_OperationCycleIdType OperationCycleId,
    uint8_t CycleState
) {
    uint16_t i;

    STD_UNUSED(OperationCycleId);

    if (!Dem_Initialized) {
        return E_NOT_OK;
    }

    if (CycleState == 1U) {  /* Cycle start */
        /* Reset TFTOC and TNCTOC for all events */
        for (i = 0U; i < DEM_MAX_NUM_EVENTS; i++) {
            Dem_EventData[i].udsStatus &= ~DEM_UDS_STATUS_TFTOC;
            Dem_EventData[i].udsStatus |= DEM_UDS_STATUS_TNCTOC;
        }
    }

    return E_OK;
}

/**
 * @brief Enable DTC setting
 */
Std_ReturnType Dem_EnableDTCSetting(void) {
    Dem_DTCSettingEnabled = TRUE;
    return E_OK;
}

/**
 * @brief Disable DTC setting
 */
Std_ReturnType Dem_DisableDTCSetting(void) {
    Dem_DTCSettingEnabled = FALSE;
    return E_OK;
}

/**
 * @brief Get number of stored events
 */
Std_ReturnType Dem_GetNumberOfEvents(uint16_t* NumberOfEvents) {
    if (NumberOfEvents == NULL_PTR) {
        return E_NOT_OK;
    }

    *NumberOfEvents = Dem_StoredEventCount;

    return E_OK;
}

/**
 * @brief Get version information
 */
void Dem_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) {
        return;
    }

    VersionInfo->vendorID = 0U;
    VersionInfo->moduleID = 54U;  /* DEM module ID */
    VersionInfo->sw_major_version = 1U;
    VersionInfo->sw_minor_version = 0U;
    VersionInfo->sw_patch_version = 0U;
}
