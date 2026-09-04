#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <json-c/json.h>

#define MAX_STACK_SIZE (1024 * 1024 * 5)

#define FILED_HOSTNAME "hostname"
#define FILED_ROOTFS_PATH "rootfs_path"
#define FILED_INTERACTIVE_SHELL "interactive_shell"
#define FILED_STACK_SIZE "stack_size"

#define DEFAULT_HOSTNAME "default-container"
#define DEFAULT_STACK_SIZE_VAL (1024 * 1024)
#define DEFAULT_INTERACTIVE_SHELL_VAL "/bin/sh"

typedef struct
{
    char hostname[256];
    char rootfs_path[1024];
    char interactive_shell[256];
    long stack_size;
} container_config;

extern char *config_path;
extern container_config global_config;

void set_config_file(char *file_name);

int load_config(const char *filename, container_config *config);
int write_config(const char *filename, container_config *config);

#endif