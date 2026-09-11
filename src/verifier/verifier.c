#include "verifier.h"
#include "config_typedefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

const char *const insecure_paths[] = {
    "/",
    "/bin",
    "/boot",
    "/dev",
    "/etc",
    "/home",
    "/lib",
    "/lib64",
    "/proc",
    "/root",
    "/sbin",
    "/sys",
    "/usr",
    "/var",
};

const size_t insecure_paths_count = sizeof(insecure_paths) / sizeof(insecure_paths[0]);


BOOL verify_config(int stack_size , const char* rootfs_path , const char* interactive_shell)
{
    if (!verify_stack_size(stack_size))
    {
        fprintf(stderr, "Invalid stack size.\n");
        return FALSE;
    }

    if (!verify_rootfs(rootfs_path , interactive_shell))
    {
        fprintf(stderr, "RootFS verification failed.\n");
        return FALSE;
    }

    if (!verify_procfs())
    {
        fprintf(stderr, "Host procfs verification failed.\n");
        return FALSE;
    }

    return TRUE;
}

BOOL verify_stack_size(int stack_size)
{
    if (stack_size <= 0)
    {
        fprintf(stderr, "[Verifier] Stack size must be positive (got %d).\n", stack_size);
        return FALSE;
    }

    if (stack_size < MIN_STACK_SIZE)
    {
        fprintf(stderr, "[Verifier] Stack size %d bytes is below the minimum of %d bytes.\n",
                stack_size, MIN_STACK_SIZE);
        return FALSE;
    }

    if (stack_size > MAX_STACK_SIZE)
    {
        fprintf(stderr, "[Verifier] Stack size %d bytes exceeds the sanity limit of %d bytes.\n",
                stack_size, MAX_STACK_SIZE);
        return FALSE;
    }

    return TRUE;
}

BOOL verify_rootfs(const char *rootfs_path , const char* interactive_shell)
{
    struct stat s;

    // Check if path is empty
    if (rootfs_path == NULL || strlen(rootfs_path) == 0)
    {
        fprintf(stderr, "[Verifier] RootFS path is empty.\n");
        return FALSE;
    }

    // Check if path exists and we can access it
    if (stat(rootfs_path, &s) != 0)
    {
        perror("[Verifier] RootFS path does not exist or is inaccessible");
        return FALSE;
    }

    // Check if it's actually a directory
    if (!S_ISDIR(s.st_mode))
    {
        fprintf(stderr, "[Verifier] RootFS path is not a directory.\n");
        return FALSE;
    }

    // Security: reject host system directories (including "/")
    if (!check_rootfs_security(rootfs_path))
        return FALSE;

    // Viability: does it have what the container needs to boot?
    if (!verify_visibility(rootfs_path , interactive_shell))
        return FALSE;

    printf("[Verifier] RootFS passed sanity checks: %s\n", rootfs_path);
    return TRUE;
}

BOOL check_rootfs_security(const char *rootfs_path)
{
    char resolved_path[PATH_MAX];

    if (realpath(rootfs_path, resolved_path) == NULL)
    {
        perror("[Verifier] Failed to resolve RootFS path");
        return FALSE;
    }

    for (size_t i = 0; i < insecure_paths_count; i++)
    {
        if (strcmp(resolved_path, insecure_paths[i]) == 0)
        {
            fprintf(stderr,
                    "[Verifier] Security Error: '%s' is a protected host directory and cannot be used as RootFS.\n",
                    resolved_path);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL verify_visibility(const char *rootfs_path , const char* interactive_shell)
{
    char sh_path[PATH_MAX];
    int written = snprintf(sh_path, sizeof(sh_path), "%s/%s", rootfs_path , interactive_shell);

    if (written < 0 || (size_t)written >= sizeof(sh_path))
    {
        fprintf(stderr, "[Verifier] RootFS path is too long.\n");
        return FALSE;
    }

    if (access(sh_path, X_OK) != 0)
    {
        fprintf(stderr, "[Verifier] RootFS is missing or cannot execute essential binary: %s\n", sh_path);
        return FALSE;
    }

    return TRUE;
}

BOOL verify_procfs(void)
{
    // Sanity-check that the host kernel exposes procfs before we rely on
    // being able to mount it inside the container later.
    struct stat s;

    if (stat("/proc/self", &s) != 0)
    {
        fprintf(stderr, "[Verifier] Host /proc is unavailable; cannot guarantee procfs support inside the container.\n");
        return FALSE;
    }

    return TRUE;
}