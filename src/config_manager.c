#include "config_manager.h"
#include <stdio.h>
#include <string.h>

BOOL config_manager_initialized = FALSE;
char *global_config_path = "";
container_config *global_config = NULL;

void initialize_config_manager(char* file_path)
{
    config_manager_initialized = TRUE;
    global_config_path = file_path;
    global_config = calloc(1 , sizeof(container_config));
}

BOOL load_config()
{
    if(config_manager_initialized == FALSE)
    {
        fprintf(stderr , "Config manager was not initialized!\n");
        return FALSE;
    }

    struct json_object *parsed_json = json_object_from_file(global_config_path);
    if (parsed_json == NULL)
    {
        fprintf(stderr, "Failed to parse JSON file or file not found: %s\n", global_config_path);
        return FALSE;
    }

    struct json_object *hostname_obj;
    struct json_object *rootfs_obj;
    struct json_object *interactive_shell;
    struct json_object *stack_size;

    // Extract the "hostname" string
    if (json_object_object_get_ex(parsed_json, FILED_HOSTNAME, &hostname_obj))
    {
        strncpy(global_config->hostname, json_object_get_string(hostname_obj), sizeof(global_config->hostname) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'hostname' key missing in config.json\n");
        printf("Using default hostname\n");
        strncpy(global_config->hostname, DEFAULT_HOSTNAME, sizeof(global_config->hostname) - 1);
    }

    // Extract the "rootfs_path" string
    if (json_object_object_get_ex(parsed_json, FILED_ROOTFS_PATH, &rootfs_obj))
    {
        strncpy(global_config->rootfs_path, json_object_get_string(rootfs_obj), sizeof(global_config->rootfs_path) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'rootfs_path' key missing in config.json\n");
        return FALSE;
    }

    // Extract the "interactive_shell" string
    if (json_object_object_get_ex(parsed_json, FILED_INTERACTIVE_SHELL, &interactive_shell))
    {
        strncpy(global_config->interactive_shell, json_object_get_string(interactive_shell), sizeof(global_config->interactive_shell) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'interactive_shell' key missing in config.json\n");
        printf("Using default interactive_shell\n");
        strncpy(global_config->interactive_shell , DEFAULT_INTERACTIVE_SHELL , sizeof(global_config->interactive_shell) - 1);
    }

    // Extract the "stack size" int
    if (json_object_object_get_ex(parsed_json, FILED_STACK_SIZE, &stack_size))
    {
        global_config->stack_size = json_object_get_int64(stack_size); // Use int64 for large stack sizes
    }
    else
    {
        fprintf(stderr, "Warning: 'stack_size' key missing in config.json.\n");
        printf("using default stack_size\n");
        global_config->stack_size = DEFAULT_STACK_SIZE;
    }

    // Free the memory allocated by the json-c library
    json_object_put(parsed_json);

    return TRUE;
}

void finish_config_manager()
{
    config_manager_initialized = FALSE;
    free(global_config);
}