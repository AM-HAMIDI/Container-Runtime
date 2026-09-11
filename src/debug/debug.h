#pragma once

#include "typedefs.h"

// Log level
typedef enum
{
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

void set_log_level(LogLevel log_level);
LogLevel get_log_level();

// Debug destination
typedef enum
{
    DISABLED,
    FILE,
    NETWORK,
    INTERACTIVE_SHELL
} DebugDest;

void set_debug_output(DebugDest debug_dest);
DebugDest get_debug_output();

// Network
typedef struct {
    const char* dest_ip;
    const char* header;
    const char* message;
} network_message;

typedef enum
{
    IPV4,
    IPV6
} ip_mode;

void set_network_dest(const char* ip);
void set_ip_mode(ip_mode mode);
char* get_network_dest();
ip_mode get_ip_mode();

#define LOG_MESSAGE_TRACE
#define LOG_MESSAGE_DEBUG
#define LOG_MESSAGE_INFO
#define LOG_MESSAGE_WARN
#define LOG_MESSAGE_ERROR
#define LOG_MESSAGE_FATAL

#define DEFAULT_LOG_FILE_PATH
#define DEFAULT_LOG_NETWORK_IP
