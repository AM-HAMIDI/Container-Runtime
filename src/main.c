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
#include "verifier.h"
#include "container.h"
#include "cgroups.h"
#include "userns.h"
#include "network.h"
#include "storage.h"

#define CLONE_FLAG (CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWUSER | CLONE_NEWNET)

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
        exit(EXIT_FAILURE);

    // Step 4 : Initialize pipe
    if (pipe(sync_pipe) != 0) 
    {
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
    container_process_struct *process_struct = calloc(1 , sizeof(container_process_struct));
    process_struct->config = global_config;
    process_struct->sync_pipe[0] = sync_pipe[0];
    process_struct->sync_pipe[1] = sync_pipe[1];
    
    int child_pid = clone(container_main, stack + global_config->stack_size,
                           CLONE_FLAG | SIGCHLD, process_struct);

    if (child_pid == -1)
    {
        perror("clone failed");
        free(stack);
        return EXIT_FAILURE;
    }

    // Step 7 : Close read end
    close(sync_pipe[0]);

    // Step 8 : Setup cgroups
    if (!setup_cgroups(global_config->hostname, child_pid, global_config->memory_limit_bytes))
    {
        fprintf(stderr, "[Host] Warning: Failed to apply cgroups.\n");
        exit(EXIT_FAILURE);
    }

    // Step 9 : Setup user mappings
    if(!setup_user_mapping(child_pid, global_config->rootfs_path))
    {   
        fprintf(stderr , "[Host] setup user mappings failed.\n");
        exit(EXIT_FAILURE);
    }

    // Step 10 : Setup network host
    if(!setup_network_host(child_pid)) {
        fprintf(stderr, "[Host] Failed to setup network.\n");
        exit(EXIT_FAILURE);
    }

    // Step 11 : Write and close the pipe
    if (write(sync_pipe[1], "0", 1) != 1) {
        fprintf(stderr, "[Host] Failed to signal child process.\n");
    }
    close(sync_pipe[1]);

    // Step 12 : Wait for container process
    waitpid(child_pid, NULL, 0);

    // Step 13 : Clean up
    clean_network_host();
    clean_storage_host();
    clean_cgroups(global_config->hostname);
    clean_config_manager();
    free(stack);

    return EXIT_SUCCESS;
}