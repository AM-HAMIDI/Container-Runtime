#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "config_handler.h"

// TODO : Dynamic stack size for each root filesystem
#define STACK_SIZE (1024 * 1024)    // 1MB stack for the cloned process
#define INTERACTIVE_SHELL "/bin/sh" // Interactive Shell for current linux

int container_main(void *arg);
void set_hostname(const char *hostname);
void isolate_fs(const char *rootfs_path);
void run_shell(void);

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
void set_hostname(const char *hostname)
{
    char *target_hostname = strlen(hostname) > 0 ? hostname : DEFAULT_CONTAINER_NAME;

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
void isolate_fs(const char *rootfs_path)
{
    // Remount the root filesystem as Private.
    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) != 0)
    {
        perror("mount / as private failed");
        return -1;
    }

    // chroot into the provided rootfs path
    if (chroot(rootfs_path) != 0)
    {
        perror("chroot failed - does the rootfs path exist?");
        return -1;
    }

    // 4. Update the current working directory to the new root
    if (chdir("/") != 0)
    {
        perror("chdir failed");
        return -1;
    }

    // 5. Mount the proc filesystem.
    // We create the /proc directory just in case the rootfs doesn't have it.
    mkdir("/proc", 0555);
    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
    {
        perror("mount procfs failed");
        return -1;
    }

    printf("[Container] Filesystem isolated securely.\n");
}

/*
    Run shell
*/
void run_shell(void)
{
    char *cmd[] = {"/bin/sh", NULL};
    execvp(cmd[0], cmd);
}

int main(int argc, char **argv)
{
    printf("[Host] Starting runtime...\n");

    // Allow user to pass a custom config path via CLI, otherwise use CMake default
    const char *config_path = (argc > 1) ? argv[1] : DEFAULT_CONFIG_PATH;
    printf("[Host] Loading configuration from: %s\n", config_path);

    container_config config = {0};
    if (load_config(config_path, &config) != 0)
    {
        fprintf(stderr, "Failed to load configuration. Exiting.\n");
        return EXIT_FAILURE;
    }

    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    int child_pid = clone(container_main,
                          stack + STACK_SIZE,
                          CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD,
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