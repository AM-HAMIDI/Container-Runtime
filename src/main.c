#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "cli.h" // Added missing cli.h
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
int run_shell(const char *interactive_shell);

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
    run_shell(config->interactive_shell);

    perror("execvp failed");
    return -1;
}

/*
    Function to set a hostname for container
*/
int set_hostname(const char *hostname)
{
// Fallback to DEFAULT_HOSTNAME defined in config_manager.h if CMake definition is missing
#ifndef DEFAULT_CONTAINER_NAME
#define DEFAULT_CONTAINER_NAME DEFAULT_HOSTNAME
#endif

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
    return F_OK; // Added missing return statement
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
int run_shell(const char *interactive_shell)
{
    // Cast to remove const warning for execvp
    char *cmd[] = {(char *)interactive_shell, NULL};
    execvp(cmd[0], cmd);

    return F_NOK; // Reached only if execvp fails
}

/*
    Main entry to container runtime
*/
int main(int argc, char **argv)
{
    // Pass args to the CLI parser
    run_cli(argc, argv);

    // Pass the global_config struct to be populated
    if (load_config(config_path, &global_config) != 0)
    {
        fprintf(stderr, "Failed to load configuration. Exiting.\n");
        return EXIT_FAILURE;
    }

    // Reference the global struct
    if (!verify_stack_size(global_config.stack_size))
    {
        fprintf(stderr, "Not valid stack size\n");
        return EXIT_FAILURE;
    }

    // Allocate stack using global struct
    char *stack = malloc(global_config.stack_size);
    if (!stack)
    {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    // Pass the populated global_config to the child process
    int child_pid = clone(container_main, stack + global_config.stack_size, CLONE_FLAG | SIGCHLD, &global_config);

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