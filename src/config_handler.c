#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "config_handler.h"

int load_config(const char *filename, container_config *config)
{
    // 1. Parse the file into a JSON object
    struct json_object *parsed_json = json_object_from_file(filename);
    if (parsed_json == NULL)
    {
        fprintf(stderr, "Failed to parse JSON file or file not found: %s\n", filename);
        return -1;
    }

    struct json_object *hostname_obj;
    struct json_object *rootfs_obj;

    // 2. Safely extract the "hostname" string
    if (json_object_object_get_ex(parsed_json, "hostname", &hostname_obj))
    {
        strncpy(config->hostname, json_object_get_string(hostname_obj), sizeof(config->hostname) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'hostname' key missing in config.json\n");
    }

    // 3. Safely extract the "rootfs_path" string
    if (json_object_object_get_ex(parsed_json, "rootfs_path", &rootfs_obj))
    {
        strncpy(config->rootfs_path, json_object_get_string(rootfs_obj), sizeof(config->rootfs_path) - 1);
    }
    else
    {
        fprintf(stderr, "Warning: 'rootfs_path' key missing in config.json\n");
    }

    // 4. Free the memory allocated by the json-c library
    json_object_put(parsed_json);

    return 0;
}

int write_config(const char *filename, container_config *config)
{
    return 0;
}