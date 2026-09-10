#include "debug.h"

// Log level
LogLevel m_log_level;

void set_log_level(LogLevel log_level)
{  
    m_log_level = log_level;
}

LogLevel get_log_level()
{
    return m_log_level;
}

// Debug destination
DebugDest m_debug_dest;
void set_debug_output(DebugDest debug_dest)
{
    m_debug_dest = debug_dest;
}

DebugDest get_debug_output()
{
    return m_debug_dest;
}

// Network
char* m_dest_ip = "";
ip_mode m_ip_mode = IPV4;

void set_network_dest(const char* ip)
{
}

char* get_network_dest()
{
    return m_dest_ip;
}

void set_ip_mode(ip_mode mode)
{
    m_ip_mode = mode;
}

ip_mode get_ip_mode()
{
    return m_ip_mode;
}