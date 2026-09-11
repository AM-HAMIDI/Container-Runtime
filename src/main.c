#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "cli.h"
#include "config_manager.h"

int sync_pipe[2];

/*
    Main entry to container runtime
*/
int main(int argc, char **argv)
{
    // Step 1 : run cli
    if(!run_cli(argc, argv))
        exit(EXIT_FAILURE);

    // Step 2 : load config
    if(!load_config())
        exit(EXIT_FAILURE);

    // Step 3 : verify config
    if(!verify_config(global_config->stack_size , global_config->rootfs_path ,
         global_config->interactive_shell))
    {
        fprintf(stderr , "Config verification failed!\n");
        return EXIT_FAILURE;
    }

    // Step 4 : Initialize pipe
    if (pipe(sync_pipe) != 0) {
        perror("pipe failed");
        return EXIT_FAILURE;
    }

    // Step 5 : initialize stack
    char *stack = malloc(global_config->stack_size);
    if (!stack)
    {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    // Step 6 : Clone container process
    int child_pid = clone(container_main, stack + global_config->stack_size,
                           CLONE_FLAG | SIGCHLD, global_config);

    if (child_pid == -1)
    {
        perror("clone failed");
        free(stack);
        return EXIT_FAILURE;
    }

    close(sync_pipe[0]);

    // Step 7 : Setup cgroups
    if (!setup_cgroups(global_config->hostname, child_pid, global_config->memory_limit_bytes))
    {
        fprintf(stderr, "[Host] Warning: Failed to apply cgroups.\n");
    }

    // Step 8 : Setup user mappings
    setup_user_mapping(child_pid, global_config->rootfs_path);

    if (write(sync_pipe[1], "0", 1) != 1) {
        fprintf(stderr, "[Host] Failed to signal child process.\n");
    }
    close(sync_pipe[1]);

    // Step 9 : Wait for container process and clean cgroups
    waitpid(child_pid, NULL, 0);
    cleanup_cgroups(global_config->hostname);

    // Step 10 : free stack
    free(stack);

    return EXIT_SUCCESS;
}