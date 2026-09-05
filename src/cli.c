#include "cli.h"
#include "config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

short relative_path_enabled = FALSE;

void enable_relative_path(void)
{
    relative_path_enabled = TRUE;
}

char *real_path(const char *relative_path)
{
}

void run_cli(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], CLI_OPTIONS_SHORT_HELP) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_HELP) == 0)
        {
            run_help();
            exit(0);
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_SET_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_SET_CONFIG_PATH) == 0)
        {
            if (i + 1 < argc)
            {
                set_config_path(argv[++i]);
            }
            else
            {
                fprintf(stderr, "Error: %s requires a path argument.\n", argv[i]);
                exit(1);
            }
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH) == 0)
        {
            set_config_file(DEFAULT_CONFIG_PATH);
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_USE_RELATIVE_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_USE_RELATIVE_PATH) == 0)
        {
            enable_relative_path();
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            run_help();
            exit(1);
        }
    }
}

void run_help(void)
{
    printf("Usage: container_runtime [OPTIONS]\n");
    printf("Options:\n");
    printf("  %s, %s\t%s\n", CLI_OPTIONS_SHORT_SET_CONFIG_PATH, CLI_OPTIONS_LONG_SET_CONFIG_PATH, CLI_MESSAGE_SET_CONFIG_PATH);
    printf("  %s, %s\t%s\n", CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH, CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH, CLI_MESSAGE_DEFAULT_CONFIG_PATH);
    printf("  %s, %s\t%s\n", CLI_OPTIONS_SHORT_USE_RELATIVE_PATH, CLI_OPTIONS_LONG_USE_RELATIVE_PATH, CLI_MESSAGE_USE_RELATIVE_PATH);
    printf("  %s, %s\t\t%s\n", CLI_OPTIONS_SHORT_HELP, CLI_OPTIONS_LONG_HELP, CLI_MESSAGE_HELP);
}