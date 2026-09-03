#pragma once

typedef enum
{
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

typedef enum
{
    FILE,
    NETWORK,
    INTERACTIVE_SHELL
} LogDest;

void export_log(LogDest logDest);