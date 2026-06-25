#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>

unsigned long get_pid_namespace_inode(pid_t pid)
{
    char path[256];
    struct stat sb;

    snprintf(path, sizeof(path), "/proc/%d/ns/pid", pid);
    if (stat(path, &sb) == -1)
    {
        perror("stat");
        return 0;
    }
    return sb.st_ino;
}

int main()
{
    printf("Before unshare - PID: %d | NS inode: %lu\n",
           getpid(), get_pid_namespace_inode(getpid()));

    if (unshare(CLONE_NEWPID) == -1)
    {
        perror("unshare(CLONE_NEWPID)");
        exit(1);
    }

    printf("After unshare (parent still old) - PID: %d | NS inode: %lu\n",
           getpid(), get_pid_namespace_inode(getpid()));

    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        exit(1);
    }

    if (child == 0)
    {
        // CHILD
        printf("\n=== CHILD ===\n");
        printf("Local PID (getpid())     : %d\n", getpid());
        printf("Namespace inode          : %lu\n", get_pid_namespace_inode(getpid()));

        // Critical: Remount /proc so it reflects the new namespace
        if (unshare(CLONE_NEWNS) == -1)
        {
            perror("unshare(CLONE_NEWNS) for /proc");
        }
        system("mount -t proc proc /proc 2>/dev/null || true");

        printf("After remounting /proc   : %lu\n", get_pid_namespace_inode(getpid()));
        sleep(2);
        return 0;
    }
    else
    {
        // PARENT
        printf("\n=== PARENT ===\n");
        printf("Local PID                : %d\n", getpid());
        printf("Namespace inode          : %lu\n", get_pid_namespace_inode(getpid()));
        printf("Child global PID         : %d\n", child);

        int status;
        wait(&status);
    }
    return 0;
}