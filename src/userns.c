#include "userns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>

static BOOL write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    if (!f) return FALSE;
    if (fprintf(f, "%s", content) < 0) {
        fclose(f);
        return FALSE;
    }
    fclose(f);
    return TRUE;
}

BOOL setup_user_mapping(int child_pid, const char *rootfs_path)
{
    char path[PATH_MAX];
    char mapping[128];
    struct stat st;
    
    // Dynamically grab the exact UID and GID of the folder owner
    if (stat(rootfs_path, &st) != 0) {
        fprintf(stderr, "[userns] Failed to stat rootfs path: %s\n", strerror(errno));
        return FALSE;
    }
    
    uid_t host_uid = st.st_uid;
    gid_t host_gid = st.st_gid;

    // 1. Write UID map
    snprintf(path, sizeof(path), "/proc/%d/uid_map", child_pid);
    snprintf(mapping, sizeof(mapping), "0 %d 1\n", host_uid);
    if (!write_file(path, mapping)) return FALSE;

    // 2. Deny setgroups
    snprintf(path, sizeof(path), "/proc/%d/setgroups", child_pid);
    if (!write_file(path, "deny\n")) return FALSE;

    // 3. Write GID map
    snprintf(path, sizeof(path), "/proc/%d/gid_map", child_pid);
    snprintf(mapping, sizeof(mapping), "0 %d 1\n", host_gid);
    if (!write_file(path, mapping)) return FALSE;

    printf("[userns] Successfully mapped container root to host UID/GID %d/%d.\n", host_uid, host_gid);
    return TRUE;
}