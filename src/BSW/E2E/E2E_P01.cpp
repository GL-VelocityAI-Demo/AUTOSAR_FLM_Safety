/**
 * @file E2E_P01.cpp
 * @brief AUTOSAR E2E Profile 01 Implementation
 * @details End-to-End communication protection Profile 01 implementation
 *          according to AUTOSAR_SWS_E2ELibrary specification.
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq02] E2E protection
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "E2E_P01.h"

/*============================================================================*
 * LOCAL MACROS
 *============================================================================*/

/** @brief Bit mask for 4-bit counter */
#define E2E_P01_COUNTER_MASK        0x0FU

/** @brief Bit mask for high nibble */
#define E2E_P01_HIGH_NIBBLE_MASK    0xF0U

/** @brief Bit mask for low nibble */
#define E2E_P01_LOW_NIBBLE_MASK     0x0FU

/*============================================================================*
 * CRC-8 LOOKUP TABLE (SAE-J1850)
 *============================================================================*/

/**
 * @brief CRC-8 lookup table for polynomial 0x1D
 * @details Pre-calculated CRC table for fast CRC computation
 */
static const uint8_t E2E_P01_CRC8_Table[256] = {
    0x00U, 0x1DU, 0x3AU, 0x27U, 0x74U, 0x69U, 0x4EU, 0x53U,
    0xE8U, 0xF5U, 0xD2U, 0xCFU, 0x9CU, 0x81U, 0xA6U, 0xBBU,
    0xCDU, 0xD0U, 0xF7U, 0xEAU, 0xB9U, 0xA4U, 0x83U, 0x9EU,
    0x25U, 0x38U, 0x1FU, 0x02U, 0x51U, 0x4CU, 0x6BU, 0x76U,
    0x87U, 0x9AU, 0xBDU, 0xA0U, 0xF3U, 0xEEU, 0xC9U, 0xD4U,
    0x6FU, 0x72U, 0x55U, 0x48U, 0x1BU, 0x06U, 0x21U, 0x3CU,
    0x4AU, 0x57U, 0x70U, 0x6DU, 0x3EU, 0x23U, 0x04U, 0x19U,
    0xA2U, 0xBFU, 0x98U, 0x85U, 0xD6U, 0xCBU, 0xECU, 0xF1U,
    0x13U, 0x0EU, 0x29U, 0x34U, 0x67U, 0x7AU, 0x5DU, 0x40U,
    0xFBU, 0xE6U, 0xC1U, 0xDCU, 0x8FU, 0x92U, 0xB5U, 0xA8U,
    0xDEU, 0xC3U, 0xE4U, 0xF9U, 0xAAU, 0xB7U, 0x90U, 0x8DU,
    0x36U, 0x2BU, 0x0CU, 0x11U, 0x42U, 0x5FU, 0x78U, 0x65U,
    0x94U, 0x89U, 0xAEU, 0xB3U, 0xE0U, 0xFDU, 0xDAU, 0xC7U,
    0x7CU, 0x61U, 0x46U, 0x5BU, 0x08U, 0x15U, 0x32U, 0x2FU,
    0x59U, 0x44U, 0x63U, 0x7EU, 0x2DU, 0x30U, 0x17U, 0x0AU,
    0xB1U, 0xACU, 0x8BU, 0x96U, 0xC5U, 0xD8U, 0xFFU, 0xE2U,
    0x26U, 0x3BU, 0x1CU, 0x01U, 0x52U, 0x4FU, 0x68U, 0x75U,
    0xCEU, 0xD3U, 0xF4U, 0xE9U, 0xBAU, 0xA7U, 0x80U, 0x9DU,
    0xEBU, 0xF6U, 0xD1U, 0xCCU, 0x9FU, 0x82U, 0xA5U, 0xB8U,
    0x03U, 0x1EU, 0x39U, 0x24U, 0x77U, 0x6AU, 0x4DU, 0x50U,
    0xA1U, 0xBCU, 0x9BU, 0x86U, 0xD5U, 0xC8U, 0xEFU, 0xF2U,
    0x49U, 0x54U, 0x73U, 0x6EU, 0x3DU, 0x20U, 0x07U, 0x1AU,
    0x6CU, 0x71U, 0x56U, 0x4BU, 0x18U, 0x05U, 0x22U, 0x3FU,
    0x84U, 0x99U, 0xBEU, 0xA3U, 0xF0U, 0xEDU, 0xCAU, 0xD7U,
    0x35U, 0x28U, 0x0FU, 0x12U, 0x41U, 0x5CU, 0x7BU, 0x66U,
    0xDDU, 0xC0U, 0xE7U, 0xFAU, 0xA9U, 0xB4U, 0x93U, 0x8EU,
    0xF8U, 0xE5U, 0xC2U, 0xDFU, 0x8CU, 0x91U, 0xB6U, 0xABU,
    0x10U, 0x0DU, 0x2AU, 0x37U, 0x64U, 0x79U, 0x5EU, 0x43U,
    0xB2U, 0xAFU, 0x88U, 0x95U, 0xC6U, 0xDBU, 0xFCU, 0xE1U,
    0x5AU, 0x47U, 0x60U, 0x7DU, 0x2EU, 0x33U, 0x14U, 0x09U,
    0x7FU, 0x62U, 0x45U, 0x58U, 0x0BU, 0x16U, 0x31U, 0x2CU,
    0x97U, 0x8AU, 0xADU, 0xB0U, 0xE3U, 0xFEU, 0xD9U, 0xC4U
};

/*============================================================================*
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static uint8_t E2E_P01_IncrementCounter(uint8_t counter);
static int16_t E2E_P01_DeltaCounter(uint8_t receivedCounter, uint8_t lastValidCounter);

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Calculate CRC-8 using SAE-J1850 polynomial
 * @details [SysSafReq02] CRC calculation for E2E protection
 */
uint8_t E2E_P01_CalculateCRC8(
    const uint8_t* data,
    uint16_t length,
    uint8_t startValue,
    boolean isFirstCall
) {
    uint8_t crc;
    uint16_t i;

    /* Input validation - defensive programming for ASIL B */
    if (data == NULL_PTR) {
        return 0xFFU;  /* Return error value */
    }

    /* Initialize CRC value */
    if (isFirstCall) {
        crc = E2E_P01_CRC_INITIAL_VALUE;
    } else {
        crc = startValue;
    }

    /* Calculate CRC using lookup table */
    for (i = 0U; i < length; i++) {
        crc = E2E_P01_CRC8_Table[crc ^ data[i]];
    }

    return crc ^ E2E_P01_CRC_XOR_VALUE;
}

/**
 * @brief Increment counter with wrap-around
 */
static uint8_t E2E_P01_IncrementCounter(uint8_t counter) {
    uint8_t newCounter = counter + 1U;

    if (newCounter > E2E_P01_COUNTER_MAX) {
        newCounter = 0U;
    }

    return newCounter;
}

/**
 * @brief Calculate delta between received and last valid counter
 */
static int16_t E2E_P01_DeltaCounter(uint8_t receivedCounter, uint8_t lastValidCounter) {
    int16_t delta;

    if (receivedCounter >= lastValidCounter) {
        delta = static_cast<int16_t>(receivedCounter) - static_cast<int16_t>(lastValidCounter);
    } else {
        /* Handle wrap-around */
        delta = static_cast<int16_t>(E2E_P01_COUNTER_WRAP) -
                static_cast<int16_t>(lastValidCounter) +
                static_cast<int16_t>(receivedCounter) + 1;
    }

    return delta;
}

/**
 * @brief Get counter value from data
 */
uint8_t E2E_P01_GetCounter(
    const uint8_t* data,
    const E2E_P01ConfigType* config
) {
    uint16_t byteOffset;
    uint8_t counter;

    if ((data == NULL_PTR) || (config == NULL_PTR)) {
        return 0U;
    }

    byteOffset = config->CounterOffset / 8U;

    /* Counter is in low nibble of the byte */
    counter = data[byteOffset] & E2E_P01_LOW_NIBBLE_MASK;

    return counter;
}

/**
 * @brief Set counter value in data
 */
void E2E_P01_SetCounter(
    uint8_t* data,
    const E2E_P01ConfigType* config,
    uint8_t counter
) {
    uint16_t byteOffset;

    if ((data == NULL_PTR) || (config == NULL_PTR)) {
        return;
    }

    byteOffset = config->CounterOffset / 8U;

    /* Preserve high nibble, set low nibble to counter */
    data[byteOffset] = (data[byteOffset] & E2E_P01_HIGH_NIBBLE_MASK) |
                       (counter & E2E_P01_LOW_NIBBLE_MASK);
}

/**
 * @brief Get CRC value from data
 */
uint8_t E2E_P01_GetCRC(
    const uint8_t* data,
    const E2E_P01ConfigType* config
) {
    uint16_t byteOffset;

    if ((data == NULL_PTR) || (config == NULL_PTR)) {
        return 0U;
    }

    byteOffset = config->CRCOffset / 8U;

    return data[byteOffset];
}

/**
 * @brief Set CRC value in data
 */
void E2E_P01_SetCRC(
    uint8_t* data,
    const E2E_P01ConfigType* config,
    uint8_t crc
) {
    uint16_t byteOffset;

    if ((data == NULL_PTR) || (config == NULL_PTR)) {
        return;
    }

    byteOffset = config->CRCOffset / 8U;

    data[byteOffset] = crc;
}

/**
 * @brief Initialize protection state
 */
Std_ReturnType E2E_P01ProtectInit(E2E_P01ProtectStateType* state) {
    if (state == NULL_PTR) {
        return E_NOT_OK;
    }

    state->Counter = 0U;

    return E_OK;
}

/**
 * @brief Protect data with E2E Profile 01
 * @details [SysSafReq02] Adds CRC and counter protection
 */
Std_ReturnType E2E_P01Protect(
    const E2E_P01ConfigType* config,
    E2E_P01ProtectStateType* state,
    uint8_t* data,
    uint16_t length
) {
    uint8_t crc;
    uint8_t dataIdBytes[2];
    uint16_t crcByteOffset;

    /* Input validation - defensive programming for ASIL B */
    if ((config == NULL_PTR) || (state == NULL_PTR) || (data == NULL_PTR)) {
        return E_NOT_OK;
    }

    if (length == 0U) {
        return E_NOT_OK;
    }

    /* Set counter in data */
    E2E_P01_SetCounter(data, config, state->Counter);

    /* Calculate CRC offset in bytes */
    crcByteOffset = config->CRCOffset / 8U;

    /* Prepare DataID bytes for CRC calculation */
    dataIdBytes[0] = static_cast<uint8_t>(config->DataID >> 8U);     /* High byte */
    dataIdBytes[1] = static_cast<uint8_t>(config->DataID & 0xFFU);  /* Low byte */

    /* Calculate CRC over DataID */
    crc = E2E_P01_CalculateCRC8(dataIdBytes, 2U, 0U, TRUE);

    /* Calculate CRC over data (excluding CRC byte) */
    if (crcByteOffset > 0U) {
        crc = E2E_P01_CalculateCRC8(data, crcByteOffset, crc, FALSE);
    }
    if (crcByteOffset < (length - 1U)) {
        crc = E2E_P01_CalculateCRC8(&data[crcByteOffset + 1U],
                                    length - crcByteOffset - 1U,
                                    crc, FALSE);
    }

    /* Set CRC in data */
    E2E_P01_SetCRC(data, config, crc);

    /* Increment counter for next transmission */
    state->Counter = E2E_P01_IncrementCounter(state->Counter);

    return E_OK;
}

/**
 * @brief Initialize check state
 */
Std_ReturnType E2E_P01CheckInit(E2E_P01CheckStateType* state) {
    if (state == NULL_PTR) {
        return E_NOT_OK;
    }

    state->LastValidCounter = 0U;
    state->MaxDeltaCounter = E2E_P01_MAX_DELTA_COUNTER_DEFAULT;
    state->WaitForFirstData = TRUE;
    state->NewDataAvailable = FALSE;
    state->LostData = 0U;
    state->Status = E2E_P01STATUS_INITIAL;
    state->NoNewOrRepeatedDataCounter = 0U;
    state->SyncCounter = 0U;

    return E_OK;
}

/**
 * @brief Check data with E2E Profile 01
 * @details [SysSafReq02] Verifies CRC and counter protection
 */
E2E_P01CheckStatusType E2E_P01Check(
    const E2E_P01ConfigType* config,
    E2E_P01CheckStateType* state,
    const uint8_t* data,
    uint16_t length
) {
    uint8_t receivedCrc;
    uint8_t calculatedCrc;
    uint8_t receivedCounter;
    int16_t deltaCounter;
    uint8_t dataIdBytes[2];
    uint16_t crcByteOffset;

    /* Input validation - defensive programming for ASIL B */
    if ((config == NULL_PTR) || (state == NULL_PTR)) {
        return E2E_P01STATUS_WRONGCRC;
    }

    /* Handle no new data case */
    if ((data == NULL_PTR) || (length == 0U)) {
        state->NoNewOrRepeatedDataCounter++;

        if (state->NoNewOrRepeatedDataCounter >= config->MaxNoNewOrRepeatedData) {
            state->Status = E2E_P01STATUS_NONEWDATA;
        }

        return state->Status;
    }

    /* Get received CRC and counter */
    receivedCrc = E2E_P01_GetCRC(data, config);
    receivedCounter = E2E_P01_GetCounter(data, config);

    /* Calculate CRC offset in bytes */
    crcByteOffset = config->CRCOffset / 8U;

    /* Prepare DataID bytes for CRC calculation */
    dataIdBytes[0] = static_cast<uint8_t>(config->DataID >> 8U);
    dataIdBytes[1] = static_cast<uint8_t>(config->DataID & 0xFFU);

    /* Calculate CRC over DataID */
    calculatedCrc = E2E_P01_CalculateCRC8(dataIdBytes, 2U, 0U, TRUE);

    /* Calculate CRC over data (excluding CRC byte) */
    if (crcByteOffset > 0U) {
        calculatedCrc = E2E_P01_CalculateCRC8(data, crcByteOffset, calculatedCrc, FALSE);
    }
    if (crcByteOffset < (length - 1U)) {
        calculatedCrc = E2E_P01_CalculateCRC8(&data[crcByteOffset + 1U],
                                              length - crcByteOffset - 1U,
                                              calculatedCrc, FALSE);
    }

    /* Verify CRC */
    if (receivedCrc != calculatedCrc) {
        state->Status = E2E_P01STATUS_WRONGCRC;
        return state->Status;
    }

    /* CRC is correct, now check counter */
    state->NoNewOrRepeatedDataCounter = 0U;

    if (state->WaitForFirstData) {
        /* First data received */
        state->WaitForFirstData = FALSE;
        state->LastValidCounter = receivedCounter;
        state->Status = E2E_P01STATUS_INITIAL;
    } else {
        /* Calculate delta counter */
        deltaCounter = E2E_P01_DeltaCounter(receivedCounter, state->LastValidCounter);

        if (deltaCounter == 0) {
            /* Repeated data */
            state->Status = E2E_P01STATUS_REPEATED;
        } else if (deltaCounter == 1) {
            /* Correct sequence */
            state->LastValidCounter = receivedCounter;
            state->Status = E2E_P01STATUS_OK;
        } else if ((deltaCounter > 1) && (deltaCounter <= static_cast<int16_t>(config->MaxDeltaCounter))) {
            /* Some data lost but within tolerance */
            state->LostData += static_cast<uint16_t>(deltaCounter - 1);
            state->LastValidCounter = receivedCounter;
            state->Status = E2E_P01STATUS_OKSOMELOST;
        } else {
            /* Wrong sequence - too many lost or negative delta */
            state->Status = E2E_P01STATUS_WRONGSEQUENCE;
        }
    }

    return state->Status;
}

/**
 * @brief Map Profile 01 status to state machine status
 */
E2E_P01CheckStatusType E2E_P01MapStatusToSM(E2E_P01CheckStatusType checkStatus) {
    return checkStatus;  /* Direct mapping for Profile 01 */
}

/**
 * @brief Initialize state machine
 */
Std_ReturnType E2E_SMCheckInit(E2E_SMCheckStateType* state) {
    if (state == NULL_PTR) {
        return E_NOT_OK;
    }

    state->OkCount = 0U;
    state->ErrorCount = 0U;
    state->SMState = E2E_SM_DEINIT;

    return E_OK;
}

/**
 * @brief Run state machine check
 * @details Implements state machine for E2E communication status
 */
E2E_SMStateType E2E_SMCheck(
    const E2E_SMConfigType* config,
    E2E_SMCheckStateType* state,
    E2E_P01CheckStatusType profileStatus
) {
    boolean isOk;

    if ((config == NULL_PTR) || (state == NULL_PTR)) {
        return E2E_SM_INVALID;
    }

    /* Determine if status is OK */
    isOk = (profileStatus == E2E_P01STATUS_OK) ||
           (profileStatus == E2E_P01STATUS_OKSOMELOST) ||
           (profileStatus == E2E_P01STATUS_INITIAL);

    /* Update counters */
    if (isOk) {
        if (state->OkCount < 0xFFU) {
            state->OkCount++;
        }
        state->ErrorCount = 0U;
    } else {
        if (state->ErrorCount < 0xFFU) {
            state->ErrorCount++;
        }
        if (profileStatus != E2E_P01STATUS_REPEATED) {
            state->OkCount = 0U;
        }
    }

    /* State machine transitions */
    switch (state->SMState) {
        case E2E_SM_DEINIT:
            state->SMState = E2E_SM_NODATA;
            break;

        case E2E_SM_NODATA:
            if (profileStatus != E2E_P01STATUS_NONEWDATA) {
                state->SMState = E2E_SM_INIT;
            }
            break;

        case E2E_SM_INIT:
            if (state->OkCount >= config->MinOkStateInit) {
                state->SMState = E2E_SM_VALID;
                state->OkCount = 0U;
                state->ErrorCount = 0U;
            } else if (state->ErrorCount >= config->MaxErrorStateInit) {
                state->SMState = E2E_SM_INVALID;
                state->OkCount = 0U;
                state->ErrorCount = 0U;
            }
            break;

        case E2E_SM_VALID:
            if (state->ErrorCount >= config->MaxErrorStateValid) {
                state->SMState = E2E_SM_INVALID;
                state->OkCount = 0U;
                state->ErrorCount = 0U;
            }
            break;

        case E2E_SM_INVALID:
            if (state->OkCount >= config->MinOkStateInvalid) {
                state->SMState = E2E_SM_VALID;
                state->OkCount = 0U;
                state->ErrorCount = 0U;
            }
            break;

        default:
            state->SMState = E2E_SM_INVALID;
            break;
    }

    return state->SMState;
}
