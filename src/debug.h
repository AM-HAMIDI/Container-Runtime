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

#define LOG_MESSAGE_TRACE
#define LOG_MESSAGE_DEBUG
#define LOG_MESSAGE_INFO
#define LOG_MESSAGE_WARN
#define LOG_MESSAGE_ERROR
#define LOG_MESSAGE_FATAL

typedef enum
{
    FILE,
    NETWORK,
    INTERACTIVE_SHELL
} LogDest;

#define LOG_FILE_PATH
#define LOG_INTERACTIVE_SHELL

void export_log(LogDest logDest);
void clean_log(void);