#ifndef CLI_H
#define CLI_H

#include "typedefs.h"

typedef enum
{
    CLI_OPT_SET_CONFIG_PATH,
    CLI_OPT_DEFAULT_CONFIG_PATH,
    CLI_OPT_USE_RELATIVE_PATH,
    CLI_OPT_HELP
} cli_options;

#define CLI_OPTIONS_SHORT_SET_CONFIG_PATH "-s"
#define CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH "-d"
#define CLI_OPTIONS_SHORT_USE_RELATIVE_PATH "-r"
#define CLI_OPTIONS_SHORT_HELP "-h"

#define CLI_OPTIONS_LONG_SET_CONFIG_PATH "--set-config"
#define CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH "--default-config"
#define CLI_OPTIONS_LONG_USE_RELATIVE_PATH "--relative-path"
#define CLI_OPTIONS_LONG_HELP "--help"

#define CLI_OPTIONS_LONG(OPTION) CLI_OPTIONS_LONG_##OPTION
#define CLI_OPTIONS_SHORT(OPTION) CLI_OPTIONS_SHORT_##OPTION

#define CLI_MESSAGE_SET_CONFIG_PATH "set config path"
#define CLI_MESSAGE_DEFAULT_CONFIG_PATH "use default config path"
#define CLI_MESSAGE_USE_RELATIVE_PATH "resolve rootfs_path relative to PATH using realpath"
#define CLI_MESSAGE_HELP "help"

#define CLI_MESSAGE(OPTION) CLI_MESSAGE_##OPTION

BOOL run_cli(int argc, char **argv);
BOOL resolve_relative_config_path(const char* path);
void run_help(void);

#endif // CLI_H