/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/** @file */

#ifndef _VT_DEBUG_H
#define _VT_DEBUG_H

#include <arduino.h>

/* Define the VT_LOG_LEVEL.  */
#ifndef VT_LOG_LEVEL
#define VT_LOG_LEVEL 2
#endif /* VT_LOG_LEVEL */

#define VTLogError(fmt, ...)
#define VTLogInfo(fmt, ...)
#define VTSpecialLog(fmt, ...)
#define VTLogDebug(fmt, ...)
#define VTLogDebugNoTag(fmt, ...)

#if VT_LOG_LEVEL > 0
#undef VTLogError
#define VTLogError(fmt, ...)                                                                                                     \
    do                                                                                                                           \
    {                                                                                                                            \
        Serial.printf("[VT ERROR] ");                                                                                                   \
        Serial.printf(fmt, ##__VA_ARGS__);                                                                                              \
    } while (0)
#endif /* VT_LOG_LEVEL > 0 */
#if VT_LOG_LEVEL > 1
#undef VTLogInfo
#define VTLogInfo(fmt, ...)                                                                                                      \
    do                                                                                                                           \
    {                                                                                                                            \
        Serial.printf("[VT INFO] ");                                                                                                    \
        Serial.printf(fmt, ##__VA_ARGS__);                                                                                              \
    } while (0)
#endif /* VT_LOG_LEVEL > 1 */

#if VT_LOG_LEVEL > 2
#undef VTSpecialLog
#define VTSpecialLog(fmt, ...)                                                                                                      \
    do                                                                                                                           \
    {                                                                                                                            \
        Serial.printf("[VT SPECIAL LOGS] ");                                                                                                    \
        Serial.printf(fmt, ##__VA_ARGS__);                                                                                              \
    } while (0)
#endif /* VT_LOG_LEVEL > 2 */

#if VT_LOG_LEVEL > 3
#undef VTLogDebug
#define VTLogDebug(fmt, ...)                                                                                                     \
    do                                                                                                                           \
    {                                                                                                                            \
        Serial.printf("[VT DEBUG] ");                                                                                                   \
        Serial.printf(fmt, ##__VA_ARGS__);                                                                                              \
    } while (0)

#undef VTLogDebugNoTag
#define VTLogDebugNoTag(fmt, ...)                                                                                                \
    do                                                                                                                           \
    {                                                                                                                            \
        Serial.printf(fmt, ##__VA_ARGS__);                                                                                              \
    } while (0)
#endif /* VT_LOG_LEVEL > 3 */

#endif
