/**
 * @file Dio.cpp
 * @brief AUTOSAR DIO Driver Implementation
 * @details MCAL DIO driver stub for simulation
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include "Dio.h"

/*============================================================================*
 * LOCAL VARIABLES
 *============================================================================*/

/** @brief Initialization flag */
static boolean Dio_Initialized = FALSE;

/** @brief Output channel levels */
static Dio_LevelType Dio_OutputLevels[DIO_NUM_CHANNELS];

/** @brief Simulated input levels */
static Dio_LevelType Dio_SimInputLevels[DIO_NUM_CHANNELS];

/** @brief Channel direction (TRUE = output, FALSE = input) */
static boolean Dio_ChannelDirection[DIO_NUM_CHANNELS];

/*============================================================================*
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/**
 * @brief Initialize DIO driver
 */
void Dio_Init(void) {
    uint8_t i;

    /* Initialize all channels to LOW */
    for (i = 0U; i < DIO_NUM_CHANNELS; i++) {
        Dio_OutputLevels[i] = STD_LOW;
        Dio_SimInputLevels[i] = STD_LOW;
        Dio_ChannelDirection[i] = FALSE;  /* Default to input */
    }

    /* Configure headlight channels as outputs */
    Dio_ChannelDirection[DIO_CHANNEL_LOW_BEAM] = TRUE;
    Dio_ChannelDirection[DIO_CHANNEL_HIGH_BEAM] = TRUE;
    Dio_ChannelDirection[DIO_CHANNEL_STATUS_LED] = TRUE;
    Dio_ChannelDirection[DIO_CHANNEL_ERROR_LED] = TRUE;

    /* Feedback is an input */
    Dio_ChannelDirection[DIO_CHANNEL_FEEDBACK] = FALSE;

    Dio_Initialized = TRUE;
}

/**
 * @brief Read DIO channel level
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId) {
    if (ChannelId >= DIO_NUM_CHANNELS) {
        return STD_LOW;
    }

    /* For outputs, return the current output level */
    /* For inputs, return the simulated input level */
    if (Dio_ChannelDirection[ChannelId]) {
        return Dio_OutputLevels[ChannelId];
    } else {
        return Dio_SimInputLevels[ChannelId];
    }
}

/**
 * @brief Write DIO channel level
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level) {
    if (ChannelId >= DIO_NUM_CHANNELS) {
        return;
    }

    /* Only write to output channels */
    if (Dio_ChannelDirection[ChannelId]) {
        Dio_OutputLevels[ChannelId] = (Level != STD_LOW) ? STD_HIGH : STD_LOW;
    }
}

/**
 * @brief Flip (toggle) DIO channel
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId) {
    Dio_LevelType currentLevel;
    Dio_LevelType newLevel;

    if (ChannelId >= DIO_NUM_CHANNELS) {
        return STD_LOW;
    }

    if (!Dio_ChannelDirection[ChannelId]) {
        /* Cannot flip input channel */
        return Dio_SimInputLevels[ChannelId];
    }

    currentLevel = Dio_OutputLevels[ChannelId];
    newLevel = (currentLevel == STD_LOW) ? STD_HIGH : STD_LOW;
    Dio_OutputLevels[ChannelId] = newLevel;

    return newLevel;
}

/**
 * @brief Read DIO port
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId) {
    Dio_PortLevelType portValue = 0U;
    uint8_t baseChannel;
    uint8_t i;

    if (PortId >= DIO_NUM_PORTS) {
        return 0U;
    }

    baseChannel = PortId * DIO_CHANNELS_PER_PORT;

    for (i = 0U; i < DIO_CHANNELS_PER_PORT; i++) {
        Dio_ChannelType ch = baseChannel + i;
        if (ch < DIO_NUM_CHANNELS) {
            Dio_LevelType level;
            if (Dio_ChannelDirection[ch]) {
                level = Dio_OutputLevels[ch];
            } else {
                level = Dio_SimInputLevels[ch];
            }
            if (level != STD_LOW) {
                portValue |= (1U << i);
            }
        }
    }

    return portValue;
}

/**
 * @brief Write DIO port
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level) {
    uint8_t baseChannel;
    uint8_t i;

    if (PortId >= DIO_NUM_PORTS) {
        return;
    }

    baseChannel = PortId * DIO_CHANNELS_PER_PORT;

    for (i = 0U; i < DIO_CHANNELS_PER_PORT; i++) {
        Dio_ChannelType ch = baseChannel + i;
        if ((ch < DIO_NUM_CHANNELS) && Dio_ChannelDirection[ch]) {
            Dio_OutputLevels[ch] = ((Level & (1U << i)) != 0U) ? STD_HIGH : STD_LOW;
        }
    }
}

/**
 * @brief Read DIO channel group
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr) {
    Dio_PortLevelType portValue;

    if (ChannelGroupIdPtr == NULL_PTR) {
        return 0U;
    }

    portValue = Dio_ReadPort(ChannelGroupIdPtr->port);

    return (portValue & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset;
}

/**
 * @brief Write DIO channel group
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr,
                           Dio_PortLevelType Level) {
    Dio_PortLevelType currentPort;
    Dio_PortLevelType newValue;

    if (ChannelGroupIdPtr == NULL_PTR) {
        return;
    }

    currentPort = Dio_ReadPort(ChannelGroupIdPtr->port);
    newValue = (currentPort & ~ChannelGroupIdPtr->mask) |
               ((Level << ChannelGroupIdPtr->offset) & ChannelGroupIdPtr->mask);

    Dio_WritePort(ChannelGroupIdPtr->port, newValue);
}

/**
 * @brief Get masked bits of port
 */
Dio_PortLevelType Dio_GetMaskedBits(Dio_PortType PortId,
                                     Dio_PortLevelType Level,
                                     Dio_PortLevelType Mask) {
    Dio_PortLevelType portValue;

    STD_UNUSED(Level);

    if (PortId >= DIO_NUM_PORTS) {
        return 0U;
    }

    portValue = Dio_ReadPort(PortId);

    return portValue & Mask;
}

/**
 * @brief Set masked bits of port
 */
void Dio_SetMaskedBits(Dio_PortType PortId,
                       Dio_PortLevelType Level,
                       Dio_PortLevelType Mask) {
    Dio_PortLevelType currentPort;
    Dio_PortLevelType newValue;

    if (PortId >= DIO_NUM_PORTS) {
        return;
    }

    currentPort = Dio_ReadPort(PortId);
    newValue = (currentPort & ~Mask) | (Level & Mask);

    Dio_WritePort(PortId, newValue);
}

/**
 * @brief Get version information
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) {
        return;
    }

    VersionInfo->vendorID = 0U;
    VersionInfo->moduleID = 120U;  /* DIO module ID */
    VersionInfo->sw_major_version = DIO_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = DIO_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = DIO_SW_PATCH_VERSION;
}

/*============================================================================*
 * SIMULATION FUNCTIONS
 *============================================================================*/

/**
 * @brief Set simulated input level
 */
void Dio_SimSetInput(Dio_ChannelType ChannelId, Dio_LevelType Level) {
    if (ChannelId >= DIO_NUM_CHANNELS) {
        return;
    }

    Dio_SimInputLevels[ChannelId] = (Level != STD_LOW) ? STD_HIGH : STD_LOW;
}

/**
 * @brief Get current output level
 */
Dio_LevelType Dio_SimGetOutput(Dio_ChannelType ChannelId) {
    if (ChannelId >= DIO_NUM_CHANNELS) {
        return STD_LOW;
    }

    return Dio_OutputLevels[ChannelId];
}

/**
 * @brief Configure channel direction
 */
void Dio_SimSetDirection(Dio_ChannelType ChannelId, boolean IsOutput) {
    if (ChannelId >= DIO_NUM_CHANNELS) {
        return;
    }

    Dio_ChannelDirection[ChannelId] = IsOutput;
}
