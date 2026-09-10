#include "cli.h"
#include "config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char relative_path_base[PATH_MAX] = {0};
static BOOL config_path_was_set = FALSE;

void resolve_relative_path(const char *path)
{
    if (realpath(path, relative_path_base) == NULL)
    {
        perror("[CLI] Failed to resolve relative path");
        exit(EXIT_FAILURE);
    }

    printf("[CLI] Relative path base resolved to: %s\n", relative_path_base);
}

void run_cli(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], CLI_OPTIONS_SHORT_HELP) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_HELP) == 0)
        {
            run_help();
            exit(EXIT_FAILURE);
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_SET_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_SET_CONFIG_PATH) == 0)
        {
            if (i + 1 < argc)
            {
                initialize_config_manager(argv[++i]);
                config_path_was_set = TRUE;
            }
            else
            {
                fprintf(stderr, "Error: %s requires a path argument.\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_DEFAULT_CONFIG_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_DEFAULT_CONFIG_PATH) == 0)
        {
            initialize_config_manager(DEFAULT_CONFIG_PATH);
            config_path_was_set = TRUE;
        }
        else if (strcmp(argv[i], CLI_OPTIONS_SHORT_USE_RELATIVE_PATH) == 0 || strcmp(argv[i], CLI_OPTIONS_LONG_USE_RELATIVE_PATH) == 0)
        {
            if (i + 1 < argc)
            {
                resolve_relative_path(argv[++i]);
                initialize_config_manager(relative_path_base);
                config_path_was_set = TRUE;
            }
            else
            {
                fprintf(stderr, "Error: %s requires a path argument.\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            run_help();
            exit(EXIT_FAILURE);
        }
    }

    if (!config_path_was_set)
    {
        initialize_config_manager(DEFAULT_CONFIG_PATH);
    }
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