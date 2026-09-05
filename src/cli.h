#ifndef CLI_H
#define CLI_H

#include "typedefs.h"

typedef enum
{
    SET_CONFIG_PATH,
    DEFAULT_CONFIG_PATH,
    USE_RELATIVE_PATH,
    HELP
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
#define CLI_MESSAGE_USE_RELATIVE_PATH "use relative path"
#define CLI_MESSAGE_HELP "help"

#define CLI_MESSAGE(OPTION) CLI_MESSAGE_##OPTION

extern short relative_path_enabled;

void run_cli(int argc, char **argv);
void enable_relative_path(void);
void run_help(void);

#endif // CLI_H