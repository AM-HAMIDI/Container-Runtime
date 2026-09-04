#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#define MAX_STACK_SIZE (1024 * 1024 * 5)

#include <json-c/json.h>

#define FILED_HOSTNAME "hostname"
#define FILED_ROOTFS_PATH "rootfs_path"
#define FILED_INTERACTIVE_SHELL "interactive_shell"
#define FILED_STACK_SIZE "stack_size"

#define DEFAULT_HOSTNAME
#define DEFAULT_STACK_SIZE
#define DEFAULT_INTERACTIVE_SHELL
#define DEFAULT_STACK_SIZE

typedef struct
{
    char hostname[256];
    char rootfs_path[1024];
    char interactive_shell[256];
    long stack_size;
} container_config;

static char *config_path = "";
static container_config config = {0};

void set_config_file(char *file_name);
int load_config(const char *filename);
int write_config(const char *filename);

#endif