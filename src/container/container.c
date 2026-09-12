#include "container.h"
#include "storage.h"
#include "seccomp.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mount.h>
#include <grp.h>

/*
    Container main start point
*/
int container_main(void *arg)
{
    container_process_struct *process_struct = (container_process_struct*) arg;

    // Initialize IPC pipeline
    if(!initialize_IPC_pipeline(process_struct))
        exit(EXIT_FAILURE);
    
    // Set privilages
    if(!set_privilages())
        exit(EXIT_FAILURE);

    // Set hostname
    if (!set_hostname(process_struct->config->hostname))
        exit(EXIT_FAILURE);

    // Set network
    if (!setup_network_container())
        exit(EXIT_FAILURE);

    // Isolate filesystem
    if (!isolate_fs(process_struct->config->rootfs_path))
        exit(EXIT_FAILURE);

    // Lock down system calls
    if (!setup_seccomp())
        exit(EXIT_FAILURE);

    // Run interactive shell
    run_shell(process_struct->config->interactive_shell);

    exit(EXIT_FAILURE);
}

BOOL initialize_IPC_pipeline(container_process_struct* process_struct)
{
    char signal;
    close(process_struct->sync_pipe[1]);
    
    if (read(process_struct->sync_pipe[0], &signal, 1) != 1) {
        fprintf(stderr, "[Container] Failed to synchronize with parent.\n");
        return FALSE;
    }
    close(process_struct->sync_pipe[0]);

    return TRUE;
}

/*
    Set mappings
*/
BOOL set_privilages(void)
{
    // // 1. Wipe inherited secondary groups
    // if (setgroups(0, NULL) != 0)
    // {
    //     perror("[Container] setgroups failed");
    //     return FALSE;
    // }

    // Set gid
    if (setgid(0) != EXIT_SUCCESS) 
    {
        perror("[Container] setgid failed");
        return FALSE;
    }

    // Set uid
    if (setuid(0) != EXIT_SUCCESS) 
    {
        perror("[Container] setuid failed");
        return FALSE;
    }

    return TRUE;
}

/*
    Set the container's hostname (falls back to DEFAULT_HOSTNAME).
*/
BOOL set_hostname(const char *hostname)
{
    const char *target_hostname =
        (hostname != NULL && strlen(hostname) > 0) ? hostname : DEFAULT_HOSTNAME;

    if (sethostname(target_hostname, strlen(target_hostname)) != 0)
    {
        perror("[Container] sethostname failed");
        return FALSE;
    }

    printf("[Container] Hostname set to: %s\n", target_hostname);
    return TRUE;
}

/*
    Isolate what the container's filesystem can read/write relative to
    the host system.
*/
BOOL isolate_fs(const char *rootfs_path)
{   
    char merged_path[1024]; 

    if (!setup_overlayfs(rootfs_path, merged_path))
    {
        perror("[Container] overlayFS failed.");
        return FALSE;
    }

    if (!mount_rootfs(rootfs_path))
    {
        perror("[Container] mount_rootfs failed");
        return FALSE;
    }

    if (!chroot_fs(merged_path))
    {
        perror("[Container] chroot_fs failed");
        return FALSE;
    }

    if (!chdir_fs())
    {
        perror("[Container] chdir_fs failed");
        return FALSE;
    }

    if (!mkdir_procfs())
    {
        perror("[Container] mkdir_procfs failed");
        return FALSE;
    }

    return TRUE;
}

/*
    Remount "/" as private+recursive so mount/unmount events don't
    propagate to or from the host.
*/
BOOL mount_rootfs(const char *rootfs_path)
{
    // Use NULL instead of "bind" for the filesystem type parameter
    if (mount(rootfs_path, rootfs_path, NULL, MS_BIND | MS_REC, NULL) != 0)
    {
        perror("[Container] bind mount rootfs failed");
        return FALSE;
    }

    // Make our new isolated mount point private
    if (mount(NULL, rootfs_path, NULL, MS_PRIVATE | MS_REC, NULL) != 0)
    {
        perror("[Container] make rootfs private failed");
        return FALSE;
    }

    return TRUE;
}

/*
    chroot into the provided rootfs path.
*/
BOOL chroot_fs(const char *rootfs_path)
{
    printf("passed rootfs_path is : %s\n" , rootfs_path);
    if (chroot(rootfs_path) != 0)
    {
        perror("[Container] chroot failed");
        return FALSE;
    }

    return TRUE;
}

/*
    Update cwd to the new root.
*/
BOOL chdir_fs(void)
{
    if (chdir("/") != 0)
    {
        perror("[Container] chdir failed");
        return FALSE;
    }

    return TRUE;
}

/*
    Mount the proc filesystem.
*/
BOOL mkdir_procfs(void)
{
    if (mkdir("/proc", 0555) != 0 && errno != EEXIST)
    {
        perror("[Container] mkdir /proc failed");
        return FALSE;
    }

    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
    {
        perror("[Container] mount procfs failed");
        return FALSE;
    }

    return TRUE;
}

/*
    Run the interactive shell, replacing the current process image.
*/
BOOL run_shell(const char *interactive_shell)
{
    char *cmd[] = {(char *)interactive_shell, NULL};
    execvp(cmd[0], cmd);

    return FALSE;
}