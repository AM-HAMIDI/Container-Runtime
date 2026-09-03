#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
// #include "typedefs.h"

// TODO : Dynamic stack size for each root filesystem
#define STACK_SIZE (1024 * 1024)    // 1MB stack for the cloned process
#define INTERACTIVE_SHELL "/bin/sh" // Interactive Shell for current linux

int container_main(void *arg);

// This function acts as the "main" for our containerized child process
int container_main(void *arg)
{
    printf("[Container] Child process started!\n");

    // Because of CLONE_NEWPID, this should print 1
    printf("[Container] My PID is: %d\n", getpid());

    // Because of CLONE_NEWUTS, changing the hostname won't affect the host
    char *new_hostname = "my-alpine-container";
    if (sethostname(new_hostname, strlen(new_hostname)) != 0)
    {
        perror("sethostname failed");
        return -1;
    }
    printf("[Container] Hostname isolated and set to: %s\n", new_hostname);

    // Execute an interactive shell.
    // Note: It's still using the host's filesystem (Step 2 will fix this)
    char *cmd[] = {"/bin/sh", NULL};
    execvp(cmd[0], cmd);

    // execvp only returns if it fails
    perror("execvp failed");
    return -1;
}

int main()
{
    printf("[Host] Starting container runtime...\n");
    printf("[Host] Host PID is: %d\n", getpid());

    // 1. Allocate memory for the child's stack on the heap
    char *stack = malloc(STACK_SIZE);
    if (stack == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // 2. Call clone() to create the isolated process
    // Flags:
    // CLONE_NEWPID: Gives the child a brand new PID tree (it becomes PID 1)
    // CLONE_NEWUTS: Gives the child a private hostname/domain name
    // SIGCHLD: Tells the kernel to send a signal to the parent when the child exits
    // Stack pointer: Points to the END of the allocated block (stack + STACK_SIZE)
    int child_pid = clone(container_main,
                          stack + STACK_SIZE,
                          CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD,
                          NULL);

    if (child_pid == -1)
    {
        perror("clone failed - (Did you run with sudo?)");
        exit(EXIT_FAILURE);
    }

    printf("[Host] Created container process with Host-level PID: %d\n", child_pid);

    // 3. Wait for the container process to exit
    waitpid(child_pid, NULL, 0);
    printf("[Host] Container exited. Cleaning up.\n");

    // 4. Clean up allocated memory
    free(stack);
    return 0;
}