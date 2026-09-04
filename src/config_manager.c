#include "config_manager.h"
#include <stdio.h>
#include <string.h>

void set_config_file(char *file_name)
{
    config_path = file_name;
}

int load_config(const char *filename, container_config *config)
{
    // Parse the file into a JSON object
    struct json_object *parsed_json = json_object_from_file(filename);
    if (parsed_json == NULL)
    {
        fprintf(stderr, "Failed to parse JSON file or file not found: %s\n", filename);
        return -1;
    }

    struct json_object *hostname_obj;
    struct json_object *rootfs_obj;
    struct json_object *interactive_shell;
    struct json_object *stack_size;

    // Extract the "hostname" string
    if (json_object_object_get_ex(parsed_json, FILED_HOSTNAME, &hostname_obj))
    {
        strncpy(config->hostname, json_object_get_string(hostname_obj), sizeof(config->hostname) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'hostname' key missing in config.json\n");
    }

    // Extract the "rootfs_path" string
    if (json_object_object_get_ex(parsed_json, FILED_ROOTFS_PATH, &rootfs_obj))
    {
        strncpy(config->rootfs_path, json_object_get_string(rootfs_obj), sizeof(config->rootfs_path) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'rootfs_path' key missing in config.json\n");
    }

    // Extract the "interactive_shell" string
    if (json_object_object_get_ex(parsed_json, FILED_INTERACTIVE_SHELL, &interactive_shell))
    {
        strncpy(config->interactive_shell, json_object_get_string(interactive_shell), sizeof(config->interactive_shell) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'interactive_shell' key missing in config.json\n");
    }

    // Extract the "stack size" int
    if (json_object_object_get_ex(parsed_json, FILED_STACK_SIZE, &stack_size))
    {
        config->stack_size = json_object_get_int(stack_size);
    }
    else
    {
        fprintf(stderr, "Warning: 'stack_size' key missing in config.json\n");
    }
    // Free the memory allocated by the json-c library
    json_object_put(parsed_json);

    return 0;
}

int write_config(const char *filename, container_config *config)
{
    return 0;
}