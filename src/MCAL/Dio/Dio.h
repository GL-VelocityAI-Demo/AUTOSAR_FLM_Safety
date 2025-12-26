/**
 * @file Dio.h
 * @brief AUTOSAR DIO Driver Interface
 * @details MCAL DIO driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef DIO_H
#define DIO_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Std_Types.h"

/*============================================================================*
 * VERSION INFORMATION
 *============================================================================*/
#define DIO_AR_RELEASE_MAJOR_VERSION        23
#define DIO_AR_RELEASE_MINOR_VERSION        11

#define DIO_SW_MAJOR_VERSION                1
#define DIO_SW_MINOR_VERSION                0
#define DIO_SW_PATCH_VERSION                0

/*============================================================================*
 * CONFIGURATION
 *============================================================================*/

/** @brief Number of DIO channels */
#define DIO_NUM_CHANNELS                    16U

/** @brief Number of DIO ports */
#define DIO_NUM_PORTS                       2U

/** @brief Channels per port */
#define DIO_CHANNELS_PER_PORT               8U

/** @brief Enable development error detection */
#define DIO_DEV_ERROR_DETECT                STD_ON

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief DIO channel type
 */
typedef uint8_t Dio_ChannelType;

/**
 * @brief DIO port type
 */
typedef uint8_t Dio_PortType;

/**
 * @brief DIO channel group type
 */
typedef struct {
    uint8_t mask;           /**< Bit mask */
    uint8_t offset;         /**< Bit offset */
    Dio_PortType port;      /**< Port ID */
} Dio_ChannelGroupType;

/**
 * @brief DIO level type
 */
typedef uint8_t Dio_LevelType;

/**
 * @brief DIO port level type
 */
typedef uint8_t Dio_PortLevelType;

/*============================================================================*
 * DIO CHANNEL DEFINITIONS
 *============================================================================*/

/** @brief Low beam relay channel */
#define DIO_CHANNEL_LOW_BEAM                0U

/** @brief High beam relay channel */
#define DIO_CHANNEL_HIGH_BEAM               1U

/** @brief Headlight feedback channel */
#define DIO_CHANNEL_FEEDBACK                2U

/** @brief Status LED channel */
#define DIO_CHANNEL_STATUS_LED              3U

/** @brief Error LED channel */
#define DIO_CHANNEL_ERROR_LED               4U

/*============================================================================*
 * FUNCTION PROTOTYPES
 *============================================================================*/

/**
 * @brief Initialize DIO driver
 */
void Dio_Init(void);

/**
 * @brief Read DIO channel level
 * @param[in] ChannelId Channel to read
 * @return Channel level (STD_HIGH/STD_LOW)
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @brief Write DIO channel level
 * @param[in] ChannelId Channel to write
 * @param[in] Level Level to set
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief Flip (toggle) DIO channel
 * @param[in] ChannelId Channel to flip
 * @return New level after flip
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

/**
 * @brief Read DIO port
 * @param[in] PortId Port to read
 * @return Port level
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @brief Write DIO port
 * @param[in] PortId Port to write
 * @param[in] Level Level to set
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief Read DIO channel group
 * @param[in] ChannelGroupIdPtr Pointer to channel group
 * @return Group level
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @brief Write DIO channel group
 * @param[in] ChannelGroupIdPtr Pointer to channel group
 * @param[in] Level Level to set
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr,
                           Dio_PortLevelType Level);

/**
 * @brief Get masked bits of port
 * @param[in] PortId Port to read
 * @param[in] Level Level to combine
 * @param[in] Mask Bit mask
 * @return Masked port level
 */
Dio_PortLevelType Dio_GetMaskedBits(Dio_PortType PortId,
                                     Dio_PortLevelType Level,
                                     Dio_PortLevelType Mask);

/**
 * @brief Set masked bits of port
 * @param[in] PortId Port to write
 * @param[in] Level Level to set
 * @param[in] Mask Bit mask
 */
void Dio_SetMaskedBits(Dio_PortType PortId,
                       Dio_PortLevelType Level,
                       Dio_PortLevelType Mask);

/**
 * @brief Get version information
 * @param[out] VersionInfo Pointer to version info
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/*============================================================================*
 * SIMULATION FUNCTIONS (FOR TESTING)
 *============================================================================*/

/**
 * @brief Set simulated input level
 * @param[in] ChannelId Channel to set
 * @param[in] Level Level to simulate
 */
void Dio_SimSetInput(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief Get current output level
 * @param[in] ChannelId Channel to read
 * @return Current output level
 */
Dio_LevelType Dio_SimGetOutput(Dio_ChannelType ChannelId);

/**
 * @brief Configure channel direction
 * @param[in] ChannelId Channel to configure
 * @param[in] IsOutput TRUE for output, FALSE for input
 */
void Dio_SimSetDirection(Dio_ChannelType ChannelId, boolean IsOutput);

#endif /* DIO_H */
