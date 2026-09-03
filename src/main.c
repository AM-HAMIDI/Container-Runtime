#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "config_handler.h"

// TODO : Dynamic stack size for each root filesystem
#define STACK_SIZE (1024 * 1024)    // 1MB stack for the cloned process
#define INTERACTIVE_SHELL "/bin/sh" // Interactive Shell for current linux

int container_main(void *arg)
{
    container_config *config = (container_config *)arg;

    printf("[Container] Process started (PID: %d)\n", getpid());

    // If the config file didn't have a hostname, use the CMake default
    char *target_hostname = strlen(config->hostname) > 0 ? config->hostname : DEFAULT_CONTAINER_NAME;

    if (sethostname(target_hostname, strlen(target_hostname)) != 0)
    {
        perror("sethostname failed");
        return -1;
    }
    printf("[Container] Hostname set to: %s\n", target_hostname);

    char *cmd[] = {"/bin/sh", NULL};
    execvp(cmd[0], cmd);

    perror("execvp failed");
    return -1;
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