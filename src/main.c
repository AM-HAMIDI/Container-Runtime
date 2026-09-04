#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "config_manager.h"
#include "typedefs.h"
#include "verifier.h"

#define CLONE_FLAG CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWUSER

int container_main(void *arg);
int set_hostname(const char *hostname);
int isolate_fs(const char *rootfs_path);
int mount_rootfs(void);
int chroot_fs(const char *rootfs_path);
int chdir_fs(void);
int mkdir_procfs(void);
int run_shell(void);

/*
    Container main start point
*/
int container_main(void *arg)
{
    container_config *config = (container_config *)arg;

    // 1 - Set a hostname for current container
    set_hostname(config->hostname);

    // 2 - Isolate filesystem
    isolate_fs(config->rootfs_path);

    // 3 - Run new shell
    run_shell();

    perror("execvp failed");
    return -1;
}

/*
    Function to set a hostname for container
*/
int set_hostname(const char *hostname)
{
    const char *target_hostname = strlen(hostname) > 0 ? hostname : DEFAULT_CONTAINER_NAME;

    if (sethostname(target_hostname, strlen(target_hostname)) != 0)
    {
        perror("sethostname failed");
        return -1;
    }
    printf("[Container] Hostname set to: %s\n", target_hostname);
    return 0;
}

/*
    This function will isolate what filesystem can read/write on the
    host system.
*/
int isolate_fs(const char *rootfs_path)
{
    if (!mount_rootfs())
        exit(EXIT_FAILURE);

    if (!chroot_fs(rootfs_path))
        exit(EXIT_FAILURE);

    if (!chdir_fs())
        exit(EXIT_FAILURE);

    if (!mkdir_procfs())
        exit(EXIT_FAILURE);

    printf("[Container] Filesystem isolated securely.\n");
}

/*
    Remount the root filesystem as Private.
*/
int mount_rootfs(void)
{
    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) != 0)
    {
        perror("mount / as private failed");
        return F_NOK;
    }

    return F_OK;
}

/*
    chroot into the provided rootfs path
*/
int chroot_fs(const char *rootfs_path)
{
    if (chroot(rootfs_path) != 0)
    {
        perror("chroot failed");
        return F_NOK;
    }

    return F_OK;
}

/*
    Update the current working directory to the new root
*/
int chdir_fs(void)
{
    if (chdir("/") != 0)
    {
        perror("chdir failed");
        return F_NOK;
    }

    return F_OK;
}

/*
    Mount the proc filesystem.
*/
int mkdir_procfs(void)
{
    mkdir("/proc", 0555);
    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
    {
        perror("mount procfs failed");
        return F_NOK;
    }

    return F_OK;
}

/*
    Run shell
*/
int run_shell(void)
{
    char *cmd[] = {"/bin/sh", NULL};
    execvp(cmd[0], cmd);
}

int main(int argc, char **argv)
{
    run_cli();

    if (load_config(config_path) != 0)
    {
        fprintf(stderr, "Failed to load configuration. Exiting.\n");
        return EXIT_FAILURE;
    }

    if (!verify_stack_size(config.stack_size))
    {
        fprintf(stderr, "Not valid stack size\n");
        return EXIT_FAILURE;
    }

    char *stack = malloc(config.stack_size);
    if (!stack)
    {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    int child_pid = clone(container_main,
                          stack + config.stack_size,
                          CLONE_FLAG | SIGCHLD,
                          &config);

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