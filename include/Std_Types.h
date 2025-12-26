/**
 * @file Std_Types.h
 * @brief AUTOSAR Standard Types Definition
 * @details This file contains the AUTOSAR standard types as defined in
 *          AUTOSAR_SWS_StandardTypes specification.
 * @version 1.0.0
 * @date 2024
 *
 * @copyright AUTOSAR Classic Platform R23-11
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/*============================================================================*
 * INCLUDES
 *============================================================================*/
#include <cstdint>
#include <cstddef>

/*============================================================================*
 * AUTOSAR VERSION INFORMATION
 *============================================================================*/
#define STD_TYPES_AR_RELEASE_MAJOR_VERSION    23
#define STD_TYPES_AR_RELEASE_MINOR_VERSION    11
#define STD_TYPES_AR_RELEASE_PATCH_VERSION    0

#define STD_TYPES_SW_MAJOR_VERSION            1
#define STD_TYPES_SW_MINOR_VERSION            0
#define STD_TYPES_SW_PATCH_VERSION            0

/*============================================================================*
 * GLOBAL SYMBOLS
 *============================================================================*/

/**
 * @brief Physical state 5V or 3.3V
 */
#define STD_HIGH        0x01U

/**
 * @brief Physical state 0V
 */
#define STD_LOW         0x00U

/**
 * @brief Logical state active
 */
#define STD_ACTIVE      0x01U

/**
 * @brief Logical state idle
 */
#define STD_IDLE        0x00U

/**
 * @brief Switch ON
 */
#define STD_ON          0x01U

/**
 * @brief Switch OFF
 */
#define STD_OFF         0x00U

/**
 * @brief Boolean true
 */
#ifndef TRUE
#define TRUE            1U
#endif

/**
 * @brief Boolean false
 */
#ifndef FALSE
#define FALSE           0U
#endif

/**
 * @brief NULL pointer definition
 */
#ifndef NULL_PTR
#define NULL_PTR        nullptr
#endif

/*============================================================================*
 * TYPE DEFINITIONS
 *============================================================================*/

/**
 * @brief Boolean type as per AUTOSAR specification
 */
typedef bool boolean;

/**
 * @brief Standard return type for AUTOSAR functions
 * @details This type is used as standard API return type.
 */
typedef uint8_t Std_ReturnType;

/**
 * @brief Return value when function executed successfully
 */
#define E_OK            0x00U

/**
 * @brief Return value when function execution failed
 */
#define E_NOT_OK        0x01U

/**
 * @brief Standard version info type
 */
typedef struct {
    uint16_t vendorID;              /**< Vendor ID */
    uint16_t moduleID;              /**< Module ID */
    uint8_t sw_major_version;       /**< Software major version */
    uint8_t sw_minor_version;       /**< Software minor version */
    uint8_t sw_patch_version;       /**< Software patch version */
} Std_VersionInfoType;

/**
 * @brief Bit ordering type for communication
 */
typedef enum {
    STD_TRANSFORMER_UNSPECIFIED = 0x00U,
    STD_TRANSFORMER_SERIALIZER  = 0x01U,
    STD_TRANSFORMER_SAFETY      = 0x02U,
    STD_TRANSFORMER_SECURITY    = 0x03U,
    STD_TRANSFORMER_CUSTOM      = 0xFFU
} Std_TransformerClass;

/**
 * @brief Error type for transformer
 */
typedef uint8_t Std_TransformerErrorCode;

#define STD_TRANSFORMER_E_OK            0x00U
#define STD_TRANSFORMER_E_SAFETY        0x01U
#define STD_TRANSFORMER_E_SECURITY      0x02U

/**
 * @brief Forward error type
 */
typedef struct {
    Std_TransformerErrorCode errorCode;
    Std_TransformerClass transformerClass;
} Std_TransformerForwardCode;

/**
 * @brief Message type for communication
 */
typedef enum {
    STD_MESSAGETYPE_REQUEST  = 0x00U,
    STD_MESSAGETYPE_RESPONSE = 0x01U
} Std_MessageTypeType;

/**
 * @brief Message result type
 */
typedef enum {
    STD_MESSAGERESULT_OK    = 0x00U,
    STD_MESSAGERESULT_ERROR = 0x01U
} Std_MessageResultType;

/*============================================================================*
 * PLATFORM SPECIFIC TYPES
 *============================================================================*/

/**
 * @brief Unsigned 8-bit integer (0..255)
 */
typedef uint8_t uint8;

/**
 * @brief Unsigned 16-bit integer (0..65535)
 */
typedef uint16_t uint16;

/**
 * @brief Unsigned 32-bit integer (0..4294967295)
 */
typedef uint32_t uint32;

/**
 * @brief Unsigned 64-bit integer
 */
typedef uint64_t uint64;

/**
 * @brief Signed 8-bit integer (-128..127)
 */
typedef int8_t sint8;

/**
 * @brief Signed 16-bit integer (-32768..32767)
 */
typedef int16_t sint16;

/**
 * @brief Signed 32-bit integer
 */
typedef int32_t sint32;

/**
 * @brief Signed 64-bit integer
 */
typedef int64_t sint64;

/**
 * @brief 32-bit floating point
 */
typedef float float32;

/**
 * @brief 64-bit floating point
 */
typedef double float64;

/*============================================================================*
 * SAFETY RELATED MACROS
 *============================================================================*/

/**
 * @brief Macro for unused parameter to avoid compiler warnings
 * @param[in] x Parameter that is intentionally unused
 */
#define STD_UNUSED(x) (void)(x)

/**
 * @brief Compile time assertion
 */
#define STD_STATIC_ASSERT(cond, msg) static_assert(cond, msg)

/**
 * @brief Maximum value for uint8
 */
#define STD_UINT8_MAX   0xFFU

/**
 * @brief Maximum value for uint16
 */
#define STD_UINT16_MAX  0xFFFFU

/**
 * @brief Maximum value for uint32
 */
#define STD_UINT32_MAX  0xFFFFFFFFUL

#endif /* STD_TYPES_H */
