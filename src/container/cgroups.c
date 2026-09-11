#include "cgroups.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>

/*
    Private helper function to write a string value into a cgroup file.
*/
static BOOL write_to_cgroup_file(const char *cgroup_path, const char *file, const char *value)
{
    char filepath[PATH_MAX];
    snprintf(filepath, sizeof(filepath), "%s/%s", cgroup_path, file);

    FILE *f = fopen(filepath, "w");
    if (!f)
    {
        fprintf(stderr, "[cgroups] Failed to open %s: %s\n", filepath, strerror(errno));
        return FALSE;
    }

    if (fprintf(f, "%s", value) < 0)
    {
        fprintf(stderr, "[cgroups] Failed to write '%s' to %s: %s\n", value, filepath, strerror(errno));
        fclose(f);
        return FALSE;
    }

    fclose(f);
    return TRUE;
}

BOOL setup_cgroups(const char *container_name, int child_pid, long memory_limit_bytes)
{
    char cgroup_path[PATH_MAX];
    snprintf(cgroup_path, sizeof(cgroup_path), "%s/%s", CGROUP_BASE_PATH, container_name);

    // Create the cgroup directory for this specific container
    // The kernel will automatically populate this with files like memory.max
    if (mkdir(cgroup_path, CGROUP_PERMISSION) != 0 && errno != EEXIST)
    {
        fprintf(stderr, "[cgroups] Failed to create cgroup directory %s: %s\n", cgroup_path, strerror(errno));
        return FALSE;
    }

    // Write the memory limit (if greater than 0)
    if (memory_limit_bytes > 0)
    {
        char mem_str[64];
        snprintf(mem_str, sizeof(mem_str), "%ld", memory_limit_bytes);
        if (!write_to_cgroup_file(cgroup_path, "memory.max", mem_str))
        {
            return FALSE;
        }
        printf("[cgroups] Memory limit restricted to %ld bytes.\n", memory_limit_bytes);
    }

    // Attach the child process to this cgroup
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", child_pid);
    if (!write_to_cgroup_file(cgroup_path, "cgroup.procs", pid_str))
    {
        return FALSE;
    }
    printf("[cgroups] PID %d secured and attached to cgroup '%s'.\n", child_pid, container_name);

    return TRUE;
}

BOOL cleanup_cgroups(const char *container_name)
{
    char cgroup_path[PATH_MAX];
    snprintf(cgroup_path, sizeof(cgroup_path), "%s/%s", CGROUP_BASE_PATH, container_name);

    // rmdir only succeeds if the directory is empty (the process must be dead)
    if (rmdir(cgroup_path) != 0 && errno != ENOENT)
    {
        fprintf(stderr, "[cgroups] Failed to remove cgroup %s: %s\n", cgroup_path, strerror(errno));
        return FALSE;
    }

    printf("[cgroups] Cleaned up kernel cgroup for '%s'.\n", container_name);
    return TRUE;
}