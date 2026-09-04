#include "verifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>

int verify_stack_size(int stack_size)
{
}

int verify_rootfs(const char *rootfs_path)
{
    struct stat s;

    // Check if path exists and we can access it
    if (stat(rootfs_path, &s) != 0)
    {
        perror("[Verifier] RootFS path does not exist or is inaccessible");
        return -1;
    }

    // Check if it's actually a directory
    if (!S_ISDIR(s.st_mode))
    {
        fprintf(stderr, "[Verifier] RootFS path is not a directory.\n");
        return -1;
    }

    // Security: Prevent accidentally using the host's actual root "/"
    // We use realpath() to resolve any "../" or symlinks to their absolute path
    char resolved_path[PATH_MAX];
    if (realpath(rootfs_path, resolved_path) != NULL)
    {
        if (strcmp(resolved_path, "/") == 0)
        {
            fprintf(stderr, "[Verifier] Security Error: RootFS cannot be the host's root directory!\n");
            return -1;
        }
    }

    // Viability: Does the target executable exist?
    // Since our runtime currently hardcodes "/bin/sh", we must ensure it is there and executable.
    char sh_path[PATH_MAX];
    snprintf(sh_path, sizeof(sh_path), "%s/bin/sh", rootfs_path);

    // access(..., X_OK) checks if the file exists AND has execute permissions
    if (access(sh_path, X_OK) != 0)
    {
        fprintf(stderr, "[Verifier] RootFS is missing or cannot execute essential binary: %s\n", sh_path);
        return -1;
    }

    printf("[Verifier] RootFS passed sanity checks: %s\n", resolved_path);
    return 0;
}

int check_rootfs_security(const char *rootfs_path)
{
    return 0;
}

int verify_visibility(const char *rootfs_path)
{
    return 0;
}

int verify_procfs(void)
{
    return 0;
}