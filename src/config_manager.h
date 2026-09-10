#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <json-c/json.h>
#include "config_typedefs.h"
#include "typedefs.h"

typedef struct
{
    char hostname[256];
    char rootfs_path[1024];
    char interactive_shell[256];
    long stack_size;
} container_config;

extern BOOL config_manager_initialized;
extern char *global_config_path;
extern container_config* global_config;

void initialize_config_manager(char* filepath);
void set_config_file_path(char *file_path);
BOOL load_config();
void finish_config_manager();

#endif