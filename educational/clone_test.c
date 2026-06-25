#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function which child process will run :
static int child_func(void *arg)
{
    printf("Child running in new PID namespace!\n");
    printf("My PID inside namespace = %d\n", getpid()); // Should be 1
    // execl("/bin/bash", "bash", NULL);
    return 12;
}

int main()
{
    // Allocate for new stack
    const int STACK_SIZE = 1024 * 1024;
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        exit(1);
    }

    // Clone : powerful generalized fork
    // can create processes + threads + new namespaces
    // arguments :
    // 1 - fn (function pointer)
    // 2 - stack : pointer to the top of the new stack
    // 3 - Flags :
    // SIGCHLD => Normal process creation (like fork)
    // CLONE_VM => Share virtual memory (threads)
    // CLONE_THREAD => Same thread group
    // CLONE_NEWPID => New PID namespace (child becomes PID 1)
    // CLONE_NEWNS => New mount namespace
    // CLONE_NEWNET => New network namespace
    // CLONE_NEWUSER => New user namespace
    // CLONE_FILES => Share file descriptor table
    pid_t pid = clone(child_func,
                      stack + STACK_SIZE,     // Stack grows downward
                      CLONE_NEWPID | SIGCHLD, // Flags
                      NULL);                  // Args for fn

    if (pid == -1)
    {
        perror("clone");
        exit(1);
    }

    printf("Parent In the global namespace created child with PID = %d\n", pid);
    int child_status;
    wait(&child_status); // Reap child
    printf("child exit status is : %d\n", WEXITSTATUS(child_status));
    free(stack);
    return 0;
}