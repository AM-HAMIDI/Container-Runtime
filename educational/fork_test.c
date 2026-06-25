#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    // After this line there will be two seprate processes :
    // 1 - Parent process : which fork() will return actual pid of child process for it
    // 2 - Child process : which fork() will return 0 as pid
    // Both will continue running this code from this line in seprate pathes
    // Both of these processes are real and we can check them using ps instruction

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        // In this case child process is running.
        // When a child process finishes (return, exit(), or killed), it doesn't disappear immediately.
        // It becomes a zombie process until the parent "reaps" (collects) its exit status.
        printf("Child process is now runnig : PID = %d, Parent PID = %d\n", getpid(), getppid());
        // getpid() : returns current process's pid
        // getppid() : returns current process's parent pid
        sleep(2);
        return 42; // Child exit code
    }
    else
    {
        // In this case parent process is running and pid returned by fork is child process pid
        pid_t child_pid = pid;
        printf("Parent process is now runnign : PID : %d , Child PID = %d\n", getpid(), child_pid);
        // Blocks (sleeps) the parent until any child process exits.
        // Stores the full status info into the status variable.
        int status;

        // -------- Option 1 : Using wait function
        // wait for any child process to finish
        // wait(&status);
        // printf("Child exited with status %d\n", WEXITSTATUS(status));

        // -------- Option 2 : Using waitpid function
        // wait for specific process to finish
        pid_t w = waitpid(child_pid, &status, 0); // 0 : blocking wait
        if (w == -1)
        {
            perror("waitpid");
        }
        else if (WIFEXITED(status))
        {
            printf("Child exited normally with status %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child was killed by signal %d\n", WTERMSIG(status));
        }
        // WEXITSTATUS(status) : Macro that extracts the actual exit code from the raw status.
    }
    return 0;
}