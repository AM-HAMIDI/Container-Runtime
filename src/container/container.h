#ifndef CONTAINER_H
#define CONTAINER_H

#include "config_manager.h"

typedef struct 
{
    int sync_pipe[2];
    container_config *config;
} container_process_struct;

int container_main(void *arg);
BOOL initialize_IPC_pipeline(container_process_struct* process_struct);
BOOL set_privilages(void);
BOOL set_hostname(const char *hostname);
BOOL isolate_fs(const char *rootfs_path);
BOOL mount_rootfs(const char *rootfs_path);
BOOL chroot_fs(const char *rootfs_path);
BOOL chdir_fs(void);
BOOL mkdir_procfs(void);
BOOL run_shell(const char *interactive_shell);


#endif // CONTAINER_H