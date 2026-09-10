#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "cli.h"
#include "config_manager.h"
#include "typedefs.h"
#include "verifier.h"
#include "cgroups.h"
#include "userns.h"

#define CLONE_FLAG (CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWUSER)

int sync_pipe[2];

int container_main(void *arg);
BOOL set_hostname(const char *hostname);
BOOL isolate_fs(const char *rootfs_path);
BOOL mount_rootfs(const char *rootfs_path);
BOOL chroot_fs(const char *rootfs_path);
BOOL chdir_fs(void);
BOOL mkdir_procfs(void);
BOOL run_shell(const char *interactive_shell);

/*
    Container main start point
*/
int container_main(void *arg)
{
    container_config *config = (container_config *)arg;

    // IPC PIPELINE
    char signal;
    close(sync_pipe[1]); // Close the write end of the pipe in the child
    
    // read() will block and pause the process here until the parent writes to the pipe
    if (read(sync_pipe[0], &signal, 1) != 1) {
        fprintf(stderr, "[Container] Failed to synchronize with parent.\n");
        exit(EXIT_FAILURE);
    }
    close(sync_pipe[0]); // Close the read end, we are done with it
    printf("[Container] Received green light from host. Booting...\n");

    if (setgid(0) != 0) {
        perror("[Container] setgid failed");
        exit(EXIT_FAILURE);
    }
    if (setuid(0) != 0) {
        perror("[Container] setuid failed");
        exit(EXIT_FAILURE);
    }
    
    if (!set_hostname(config->hostname))
    {
        fprintf(stderr, "[Container] Failed to set hostname. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    if (!isolate_fs(config->rootfs_path))
    {
        fprintf(stderr, "[Container] Failed to isolate filesystem. Aborting.\n");
        exit(EXIT_FAILURE);
    }

    run_shell(config->interactive_shell);

    perror("[Container] execvp failed");
    exit(EXIT_FAILURE);
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
    if (!mount_rootfs(rootfs_path))
        return FALSE;

    if (!chroot_fs(rootfs_path))
        return FALSE;

    if (!chdir_fs())
        return FALSE;

    if (!mkdir_procfs())
        return FALSE;

    printf("[Container] Filesystem isolated securely.\n");
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

    // Only reached if execvp failed.
    return FALSE;
}

/*
    Main entry to container runtime
*/
int main(int argc, char **argv)
{
    run_cli(argc, argv);

    if (load_config() == FALSE)
    {
        fprintf(stderr, "Failed to load configuration. Exiting.\n");
        return EXIT_FAILURE;
    }

    if(!verify_config(global_config->stack_size , global_config->rootfs_path ,
         global_config->interactive_shell))
    {
        fprintf(stderr , "Config verification failed!\n");
        return EXIT_FAILURE;
    }

    // NEW: Initialize the pipe before cloning
    if (pipe(sync_pipe) != 0) {
        perror("pipe failed");
        return EXIT_FAILURE;
    }

    char *stack = malloc(global_config->stack_size);
    if (!stack)
    {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    int child_pid = clone(container_main, stack + global_config->stack_size,
                           CLONE_FLAG | SIGCHLD, global_config);

    if (child_pid == -1)
    {
        perror("clone failed");
        free(stack);
        return EXIT_FAILURE;
    }

    printf("[Host] Parent Runtime PID: %d\n", getpid());
    printf("[Host] Spawned Container (Child) PID: %d\n", child_pid);

    close(sync_pipe[0]); // Close the read end of the pipe in the parent

    if (!setup_cgroups(global_config->hostname, child_pid, global_config->memory_limit_bytes))
    {
        fprintf(stderr, "[Host] Warning: Failed to apply cgroups.\n");
    }

    setup_user_mapping(child_pid, global_config->rootfs_path);

    if (write(sync_pipe[1], "0", 1) != 1) {
        fprintf(stderr, "[Host] Failed to signal child process.\n");
    }
    close(sync_pipe[1]); // Close the write end

    waitpid(child_pid, NULL, 0);
    cleanup_cgroups(global_config->hostname);

    printf("[Host] Container exited.\n");

    free(stack);
    return 0;
}