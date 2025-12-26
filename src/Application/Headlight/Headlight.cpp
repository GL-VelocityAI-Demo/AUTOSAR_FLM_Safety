/**
 * @file Headlight.cpp
 * @brief Headlight SWC Implementation
 * @details Controls headlight output with feedback monitoring
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq10] Output stage diagnosis
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Headlight.h"
#include "Application/FLM/FLM_Application.h"
#include "MCAL/Dio/Dio.h"
#include "MCAL/Adc/Adc.h"
#include "Dem_Cfg.h"
#include <cstring>

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Component internal state */
static Headlight_StateType Headlight_State;

/** @brief System time counter */
static uint32_t Headlight_SystemTime = 0U;

/** @brief Simulated feedback current */
static uint16_t Headlight_SimCurrent = 0U;
static boolean Headlight_SimCurrentEnabled = FALSE;

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void Headlight_ReadFeedback(void);
static void Headlight_SetOutputs(void);
static void Headlight_CheckOpenLoad(void);
static void Headlight_CheckShortCircuit(void);
static void Headlight_UpdateFaultStatus(void);
static void Headlight_ReportDemEvents(void);
static boolean Headlight_IsOutputCommanded(void);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize Headlight component
 */
void Headlight_Init(void) {
    /* Clear state structure */
    (void)memset(&Headlight_State, 0, sizeof(Headlight_State));

    /* Initialize command */
    Headlight_State.currentCommand = HEADLIGHT_CMD_OFF;
    Headlight_State.requestedCommand = HEADLIGHT_CMD_OFF;
    Headlight_State.lowBeamOutput = FALSE;
    Headlight_State.highBeamOutput = FALSE;

    /* Initialize feedback */
    Headlight_State.feedbackCurrent = 0U;
    Headlight_State.feedbackState = FALSE;

    /* Initialize fault detection */
    Headlight_State.faultStatus = HEADLIGHT_FAULT_NONE;
    Headlight_State.openLoadCounter = 0U;
    Headlight_State.shortCircuitCounter = 0U;
    Headlight_State.faultConfirmed = FALSE;

    /* Initialize DIO outputs to OFF */
    Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_LOW);
    Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_LOW);

    /* Mark as initialized */
    Headlight_State.isInitialized = TRUE;
}

/**
 * @brief Main function for Headlight
 * @details [SysSafReq10] Output stage diagnosis within 20ms
 */
void Headlight_MainFunction(void) {
    if (!Headlight_State.isInitialized) {
        return;
    }

    /* Update timestamp */
    Headlight_State.currentTime = Headlight_SystemTime;
    Headlight_SystemTime += FLM_MAIN_FUNCTION_PERIOD_MS;

    /* Get command from FLM */
    Headlight_State.requestedCommand = FLM_GetHeadlightCommand();

    /* Set physical outputs */
    Headlight_SetOutputs();

    /* Read feedback current */
    Headlight_ReadFeedback();

    /* Check for open load [SysSafReq10] */
    Headlight_CheckOpenLoad();

    /* Check for short circuit [SysSafReq10] */
    Headlight_CheckShortCircuit();

    /* Update overall fault status */
    Headlight_UpdateFaultStatus();

    /* Report DEM events */
    Headlight_ReportDemEvents();

    /* Update current command */
    Headlight_State.currentCommand = Headlight_State.requestedCommand;
}

/**
 * @brief Set physical outputs based on command
 */
static void Headlight_SetOutputs(void) {
    switch (Headlight_State.requestedCommand) {
        case HEADLIGHT_CMD_OFF:
            Headlight_State.lowBeamOutput = FALSE;
            Headlight_State.highBeamOutput = FALSE;
            Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_LOW);
            Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_LOW);
            break;

        case HEADLIGHT_CMD_LOW_BEAM:
            Headlight_State.lowBeamOutput = TRUE;
            Headlight_State.highBeamOutput = FALSE;
            Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_HIGH);
            Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_LOW);
            break;

        case HEADLIGHT_CMD_HIGH_BEAM:
            Headlight_State.lowBeamOutput = TRUE;
            Headlight_State.highBeamOutput = TRUE;
            Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_HIGH);
            Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_HIGH);
            break;

        default:
            /* Invalid command - turn off for safety */
            Headlight_State.lowBeamOutput = FALSE;
            Headlight_State.highBeamOutput = FALSE;
            Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_LOW);
            Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_LOW);
            break;
    }

    /* Record command change time */
    if (Headlight_State.requestedCommand != Headlight_State.currentCommand) {
        Headlight_State.commandChangeTime = Headlight_State.currentTime;
    }
}

/**
 * @brief Read feedback current from ADC
 */
static void Headlight_ReadFeedback(void) {
    Adc_ValueGroupType adcValue;
    Std_ReturnType result;

    /* Check if using simulated value */
    if (Headlight_SimCurrentEnabled) {
        Headlight_State.feedbackCurrent = Headlight_SimCurrent;
    } else {
        /* Read current sense ADC */
        Adc_StartGroupConversion(HEADLIGHT_ADC_CURRENT_SENSE);
        result = Adc_ReadGroup(HEADLIGHT_ADC_CURRENT_SENSE, &adcValue);

        if (result == E_OK) {
            /* Convert ADC to current (mA) */
            Headlight_State.feedbackCurrent =
                adcValue * FLM_HEADLIGHT_CURRENT_FACTOR;
        }
    }

    /* Determine feedback state */
    Headlight_State.feedbackState =
        (Headlight_State.feedbackCurrent >= FLM_HEADLIGHT_MIN_CURRENT_MA);
}

/**
 * @brief Check for open load condition
 * @details [SysSafReq10] Detect when commanded ON but no current flows
 */
static void Headlight_CheckOpenLoad(void) {
    uint32_t timeSinceCommand;

    /* Only check if output is commanded ON */
    if (!Headlight_IsOutputCommanded()) {
        Headlight_State.openLoadCounter = 0U;
        return;
    }

    /* Wait for settling time after command change */
    timeSinceCommand = Headlight_State.currentTime -
                       Headlight_State.commandChangeTime;

    if (timeSinceCommand < FLM_HEADLIGHT_FAULT_DETECT_MS) {
        return;  /* Still settling */
    }

    /* Check for open load: commanded ON but no current */
    if (Headlight_State.feedbackCurrent < FLM_HEADLIGHT_OPEN_LOAD_MA) {
        Headlight_State.openLoadCounter++;

        if (Headlight_State.openLoadCounter >= HEADLIGHT_FAULT_CONFIRM_CYCLES) {
            Headlight_State.faultStatus = HEADLIGHT_FAULT_OPEN_LOAD;
            Headlight_State.faultConfirmed = TRUE;
        }
    } else {
        Headlight_State.openLoadCounter = 0U;
    }
}

/**
 * @brief Check for short circuit condition
 * @details [SysSafReq10] Detect overcurrent condition
 */
static void Headlight_CheckShortCircuit(void) {
    /* Check for overcurrent regardless of command */
    if (Headlight_State.feedbackCurrent > FLM_HEADLIGHT_MAX_CURRENT_MA) {
        Headlight_State.shortCircuitCounter++;

        if (Headlight_State.shortCircuitCounter >= HEADLIGHT_FAULT_CONFIRM_CYCLES) {
            Headlight_State.faultStatus = HEADLIGHT_FAULT_SHORT;
            Headlight_State.faultConfirmed = TRUE;

            /* Immediately turn off outputs for protection */
            Dio_WriteChannel(HEADLIGHT_DIO_LOW_BEAM, STD_LOW);
            Dio_WriteChannel(HEADLIGHT_DIO_HIGH_BEAM, STD_LOW);
            Headlight_State.lowBeamOutput = FALSE;
            Headlight_State.highBeamOutput = FALSE;
        }
    } else {
        Headlight_State.shortCircuitCounter = 0U;
    }
}

/**
 * @brief Update overall fault status
 */
static void Headlight_UpdateFaultStatus(void) {
    /* If no faults detected and counters are zero, clear fault */
    if ((Headlight_State.openLoadCounter == 0U) &&
        (Headlight_State.shortCircuitCounter == 0U)) {
        if (!Headlight_State.faultConfirmed) {
            Headlight_State.faultStatus = HEADLIGHT_FAULT_NONE;
        }
        /* Confirmed faults remain until reset */
    }
}

/**
 * @brief Check if any output is commanded ON
 */
static boolean Headlight_IsOutputCommanded(void) {
    return (Headlight_State.requestedCommand != HEADLIGHT_CMD_OFF);
}

/**
 * @brief Report events to DEM
 */
static void Headlight_ReportDemEvents(void) {
    /* Report open load */
    if (Headlight_State.faultStatus == HEADLIGHT_FAULT_OPEN_LOAD) {
        (void)Rte_Call_Headlight_Dem_SetEventStatus(
            DEM_EVENT_HEADLIGHT_OPEN_LOAD,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_Headlight_Dem_SetEventStatus(
            DEM_EVENT_HEADLIGHT_OPEN_LOAD,
            DEM_EVENT_STATUS_PASSED
        );
    }

    /* Report short circuit */
    if (Headlight_State.faultStatus == HEADLIGHT_FAULT_SHORT) {
        (void)Rte_Call_Headlight_Dem_SetEventStatus(
            DEM_EVENT_HEADLIGHT_SHORT_CIRCUIT,
            DEM_EVENT_STATUS_FAILED
        );
    } else {
        (void)Rte_Call_Headlight_Dem_SetEventStatus(
            DEM_EVENT_HEADLIGHT_SHORT_CIRCUIT,
            DEM_EVENT_STATUS_PASSED
        );
    }
}

/*============================================================================*
 * PUBLIC INTERFACE FUNCTIONS
 *============================================================================*/

void Headlight_SetCommand(HeadlightCommand cmd) {
    Headlight_State.requestedCommand = cmd;
}

HeadlightFaultStatus Headlight_GetFaultStatus(void) {
    return Headlight_State.faultStatus;
}

boolean Headlight_GetActualState(void) {
    return Headlight_State.feedbackState;
}

HeadlightCommand Headlight_GetCurrentCommand(void) {
    return Headlight_State.currentCommand;
}

void Headlight_SimSetFeedbackCurrent(uint16_t current) {
    Headlight_SimCurrent = current;
    Headlight_SimCurrentEnabled = TRUE;
}

const Headlight_StateType* Headlight_GetState(void) {
    return &Headlight_State;
}

/*============================================================================*
 * RTE PORT IMPLEMENTATIONS (STUBS)
 *============================================================================*/

Rte_StatusType Rte_Read_Headlight_Command(HeadlightCommand* command) {
    if (command != NULL_PTR) {
        *command = Headlight_State.requestedCommand;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Write_Headlight_FaultStatus(HeadlightFaultStatus status) {
    STD_UNUSED(status);
    return RTE_E_OK;
}

Rte_StatusType Rte_Write_Headlight_ActualState(boolean state) {
    STD_UNUSED(state);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_Headlight_Dio_WriteChannel(
    uint8_t channelId,
    uint8_t level
) {
    Dio_WriteChannel(channelId, level);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_Headlight_Dio_ReadChannel(
    uint8_t channelId,
    uint8_t* level
) {
    if (level != NULL_PTR) {
        *level = Dio_ReadChannel(channelId);
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

Rte_StatusType Rte_Call_Headlight_Adc_GetCurrentSense(
    uint8_t channel,
    uint16_t* value
) {
    Adc_ValueGroupType adcValue;
    if (Adc_ReadGroup(channel, &adcValue) == E_OK) {
        *value = adcValue;
        return RTE_E_OK;
    }
    return RTE_E_NO_DATA;
}

Rte_StatusType Rte_Call_Headlight_WdgM_CheckpointReached(
    WdgM_SupervisedEntityIdType SEId,
    WdgM_CheckpointIdType CPId
) {
    STD_UNUSED(SEId);
    STD_UNUSED(CPId);
    return RTE_E_OK;
}

Rte_StatusType Rte_Call_Headlight_Dem_SetEventStatus(
    uint16_t eventId,
    Dem_EventStatusType eventStatus
) {
    STD_UNUSED(eventId);
    STD_UNUSED(eventStatus);
    return RTE_E_OK;
}

Rte_TimestampType Rte_IrvRead_Headlight_SystemTime(void) {
    return Headlight_SystemTime;
}
