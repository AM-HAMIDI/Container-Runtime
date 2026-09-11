#include "cli.h"
#include "config_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BOOL resolve_relative_config_path(const char* path)
{
    char full_path[PATH_MAX];
    if (realpath(path, full_path) == NULL)
    {
        perror("[CLI] Failed to resolve relative path");
        return FALSE;
    }

    initialize_config_manager(full_path);
    return TRUE;
}

BOOL run_cli(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        // Case 1 : Help
        if (strcmp(argv[i], CLI_OPTIONS_SHORT_HELP) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_HELP) == 0)
        {
            run_help();
            return FALSE;
        }
        // Case 2 : Set config path
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_SET_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_SET_CONFIG_PATH) == 0)
        {
            if (i + 1 < argc)
            {
                initialize_config_manager(argv[++i]);
                return TRUE;
            }
            else
            {
                fprintf(stderr, "Error: %s requires a path argument.\n", argv[i]);
                return FALSE;
            }
        }
        // Case 3 : Set config default path 
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH) == 0)
        {
            initialize_config_manager(DEFAULT_CONFIG_PATH);
            return TRUE;
        }
        // Case 4 : Set relative path
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_USE_RELATIVE_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_USE_RELATIVE_PATH) == 0)
        {
            if (i + 1 < argc)
            {
                return resolve_relative_path(argv[++i]);
            }
            else
            {
                fprintf(stderr, "Error: %s requires a path argument.\n", argv[i]);
                return FALSE;
            }
        }
        // Case 5 : Unknown option
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            run_help();
            return FALSE;
        }
    }

    initialize_config_manager(DEFAULT_CONFIG_PATH);
    return TRUE;
}

void run_help(void)
{
    printf("Usage: container_runtime [OPTIONS]\n");
    printf("Options:\n");
    printf("  %s, %s <path>\t%s\n", CLI_OPTIONS_SHORT_SET_CONFIG_PATH, CLI_OPTIONS_LONG_SET_CONFIG_PATH, CLI_MESSAGE_SET_CONFIG_PATH);
    printf("  %s, %s\t%s\n", CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH, CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH, CLI_MESSAGE_DEFAULT_CONFIG_PATH);
    printf("  %s, %s <path>\t%s\n", CLI_OPTIONS_SHORT_USE_RELATIVE_PATH, CLI_OPTIONS_LONG_USE_RELATIVE_PATH, CLI_MESSAGE_USE_RELATIVE_PATH);
    printf("  %s, %s\t\t%s\n", CLI_OPTIONS_SHORT_HELP, CLI_OPTIONS_LONG_HELP, CLI_MESSAGE_HELP);
}