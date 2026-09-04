#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H

typedef struct
{
    char hostname[256];
    char rootfs_path[1024];
} container_config;

int load_config(const char *filename, container_config *config);
int write_config(const char *filename, container_config *config);

#endif