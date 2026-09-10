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

#define CLONE_FLAG (CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS)

int container_main(void *arg);
BOOL set_hostname(const char *hostname);
BOOL isolate_fs(const char *rootfs_path);
BOOL mount_rootfs(void);
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
    if (!mount_rootfs())
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
BOOL mount_rootfs(void)
{
    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) != 0)
    {
        perror("[Container] mount / as private failed");
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

    waitpid(child_pid, NULL, 0);
    printf("[Host] Container exited.\n");

    free(stack);
    return 0;
}