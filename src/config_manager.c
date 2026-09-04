#include "config_manager.h"
#include <stdio.h>
#include <string.h>

// Define the global variables here
char *config_path = "";
container_config global_config = {0};

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
        config->stack_size = json_object_get_int64(stack_size); // Use int64 for large stack sizes
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
    // Create a new JSON object
    struct json_object *root = json_object_new_object();
    if (!root)
    {
        fprintf(stderr, "Failed to allocate memory for JSON object\n");
        return -1;
    }

    // Add fields from the struct to the JSON object
    json_object_object_add(root, FILED_HOSTNAME, json_object_new_string(config->hostname));
    json_object_object_add(root, FILED_ROOTFS_PATH, json_object_new_string(config->rootfs_path));
    json_object_object_add(root, FILED_INTERACTIVE_SHELL, json_object_new_string(config->interactive_shell));
    json_object_object_add(root, FILED_STACK_SIZE, json_object_new_int64(config->stack_size));

    // Write the JSON object to the specified file
    if (json_object_to_file_ext(filename, root, JSON_C_TO_STRING_PRETTY) < 0)
    {
        fprintf(stderr, "Failed to write config to %s\n", filename);
        json_object_put(root); // Free memory before returning
        return -1;
    }

    // Free the memory allocated by the json-c library
    json_object_put(root);
    return 0;
}