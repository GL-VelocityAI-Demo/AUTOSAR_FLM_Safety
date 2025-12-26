/**
 * @file E2E_P01.h
 * @brief AUTOSAR E2E Profile 01 Interface
 * @details End-to-End communication protection Profile 01 implementation
 *          according to AUTOSAR_SWS_E2ELibrary specification.
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 *
 * @safety ASIL B - [SysSafReq02] E2E protection
 */

#ifndef E2E_P01_H
#define E2E_P01_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"
#include "Rte/Rte_Type.h"

/*============================================================================*
 * VERSION INFORMATION
 *============================================================================*/
#define E2E_P01_AR_RELEASE_MAJOR_VERSION    23
#define E2E_P01_AR_RELEASE_MINOR_VERSION    11

#define E2E_P01_SW_MAJOR_VERSION            1
#define E2E_P01_SW_MINOR_VERSION            0
#define E2E_P01_SW_PATCH_VERSION            0

/*============================================================================*
 * CONFIGURATION CONSTANTS
 *============================================================================*/

/** @brief CRC-8 Polynomial (SAE-J1850) */
#define E2E_P01_CRC_POLYNOMIAL              0x1DU

/** @brief CRC-8 Initial value */
#define E2E_P01_CRC_INITIAL_VALUE           0xFFU

/** @brief CRC-8 XOR output value */
#define E2E_P01_CRC_XOR_VALUE               0xFFU

/** @brief Maximum counter value (4-bit: 0-14) */
#define E2E_P01_COUNTER_MAX                 14U

/** @brief Counter wrap value */
#define E2E_P01_COUNTER_WRAP                15U

/** @brief Default maximum delta counter */
#define E2E_P01_MAX_DELTA_COUNTER_DEFAULT   1U

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief E2E Profile 01 Configuration Type
 * @details Configuration structure for E2E Profile 01
 */
typedef struct {
    uint16_t DataLength;              /**< Data length in bits (including CRC and Counter) */
    uint16_t DataID;                  /**< Unique identifier for the data element */
    uint8_t MaxDeltaCounter;          /**< Maximum allowed counter jump */
    uint16_t MaxNoNewOrRepeatedData;  /**< Maximum cycles without new data */
    uint16_t SyncCounter;             /**< Number of checks to be synchronized */
    uint16_t CounterOffset;           /**< Bit position of counter in data */
    uint16_t CRCOffset;               /**< Bit position of CRC in data */
    uint16_t DataIDNibbleOffset;      /**< Offset of DataID nibble */
    boolean DataIDMode;               /**< FALSE: Two bytes, TRUE: Low nibble */
} E2E_P01ConfigType;

/**
 * @brief E2E Profile 01 Protection State Type
 * @details State maintained during E2E protection (sender side)
 */
typedef struct {
    uint8_t Counter;                  /**< Current counter value (0-14) */
} E2E_P01ProtectStateType;

/**
 * @brief E2E Profile 01 Check State Type
 * @details State maintained during E2E check (receiver side)
 */
typedef struct {
    uint8_t LastValidCounter;         /**< Last valid counter received */
    uint8_t MaxDeltaCounter;          /**< Maximum delta counter allowed */
    boolean WaitForFirstData;         /**< Waiting for first valid data */
    boolean NewDataAvailable;         /**< New data has been received */
    uint16_t LostData;                /**< Count of lost data */
    E2E_P01CheckStatusType Status;    /**< Last check status */
    uint16_t NoNewOrRepeatedDataCounter;  /**< Counter for no new/repeated data */
    uint16_t SyncCounter;             /**< Synchronization counter */
} E2E_P01CheckStateType;

/**
 * @brief E2E State Machine Configuration
 */
typedef struct {
    uint8_t WindowSize;               /**< Window size for state machine */
    uint8_t MinOkStateInit;           /**< Min OK states for INIT->VALID */
    uint8_t MaxErrorStateInit;        /**< Max error states for INIT->INVALID */
    uint8_t MinOkStateValid;          /**< Min OK states to stay VALID */
    uint8_t MinOkStateInvalid;        /**< Min OK states for INVALID->VALID */
    uint8_t MaxErrorStateValid;       /**< Max error states for VALID->INVALID */
    uint8_t MaxErrorStateInvalid;     /**< Max error states to stay INVALID */
} E2E_SMConfigType;

/**
 * @brief E2E State Machine State Type
 */
typedef struct {
    uint8_t OkCount;                  /**< Count of OK statuses in window */
    uint8_t ErrorCount;               /**< Count of error statuses in window */
    E2E_SMStateType SMState;          /**< Current state machine state */
} E2E_SMCheckStateType;

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize protection state
 * @param[out] state Pointer to protection state to initialize
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType E2E_P01ProtectInit(E2E_P01ProtectStateType* state);

/**
 * @brief Protect data with E2E Profile 01
 * @details Adds CRC and counter to data
 *          [SysSafReq02] E2E protection
 * @param[in] config Pointer to configuration
 * @param[in,out] state Pointer to protection state
 * @param[in,out] data Pointer to data buffer
 * @param[in] length Data length in bytes
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType E2E_P01Protect(
    const E2E_P01ConfigType* config,
    E2E_P01ProtectStateType* state,
    uint8_t* data,
    uint16_t length
);

/**
 * @brief Initialize check state
 * @param[out] state Pointer to check state to initialize
 * @return E_OK on success, E_NOT_OK on failure
 */
Std_ReturnType E2E_P01CheckInit(E2E_P01CheckStateType* state);

/**
 * @brief Check data with E2E Profile 01
 * @details Verifies CRC and counter in received data
 *          [SysSafReq02] E2E protection
 * @param[in] config Pointer to configuration
 * @param[in,out] state Pointer to check state
 * @param[in] data Pointer to received data
 * @param[in] length Data length in bytes
 * @return Check status indicating result
 */
E2E_P01CheckStatusType E2E_P01Check(
    const E2E_P01ConfigType* config,
    E2E_P01CheckStateType* state,
    const uint8_t* data,
    uint16_t length
);

/**
 * @brief Map P01 status to state machine status
 * @param[in] checkStatus Status from E2E_P01Check
 * @return Mapped status for state machine
 */
E2E_P01CheckStatusType E2E_P01MapStatusToSM(
    E2E_P01CheckStatusType checkStatus
);

/**
 * @brief Initialize state machine
 * @param[out] state Pointer to SM state to initialize
 * @return E_OK on success
 */
Std_ReturnType E2E_SMCheckInit(E2E_SMCheckStateType* state);

/**
 * @brief Run state machine check
 * @param[in] config Pointer to SM configuration
 * @param[in,out] state Pointer to SM state
 * @param[in] profileStatus Status from profile check
 * @return Updated state machine status
 */
E2E_SMStateType E2E_SMCheck(
    const E2E_SMConfigType* config,
    E2E_SMCheckStateType* state,
    E2E_P01CheckStatusType profileStatus
);

/**
 * @brief Calculate CRC-8 (SAE-J1850)
 * @param[in] data Pointer to data
 * @param[in] length Data length in bytes
 * @param[in] startValue Start value for CRC calculation
 * @param[in] isFirstCall TRUE if first call (for segmented calculation)
 * @return Calculated CRC-8 value
 */
uint8_t E2E_P01_CalculateCRC8(
    const uint8_t* data,
    uint16_t length,
    uint8_t startValue,
    boolean isFirstCall
);

/**
 * @brief Get counter from data
 * @param[in] data Pointer to data
 * @param[in] config Pointer to configuration
 * @return Counter value
 */
uint8_t E2E_P01_GetCounter(
    const uint8_t* data,
    const E2E_P01ConfigType* config
);

/**
 * @brief Set counter in data
 * @param[in,out] data Pointer to data
 * @param[in] config Pointer to configuration
 * @param[in] counter Counter value to set
 */
void E2E_P01_SetCounter(
    uint8_t* data,
    const E2E_P01ConfigType* config,
    uint8_t counter
);

/**
 * @brief Get CRC from data
 * @param[in] data Pointer to data
 * @param[in] config Pointer to configuration
 * @return CRC value
 */
uint8_t E2E_P01_GetCRC(
    const uint8_t* data,
    const E2E_P01ConfigType* config
);

/**
 * @brief Set CRC in data
 * @param[in,out] data Pointer to data
 * @param[in] config Pointer to configuration
 * @param[in] crc CRC value to set
 */
void E2E_P01_SetCRC(
    uint8_t* data,
    const E2E_P01ConfigType* config,
    uint8_t crc
);

#endif /* E2E_P01_H */
