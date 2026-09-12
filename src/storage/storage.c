#include "storage.h"
#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BOOL setup_overlayfs(const char *base_rootfs, char *merged_path_out) {
    // Create ephemeral directories in the host's /tmp before chrooting
    if (system("mkdir -p /tmp/runtime/upper /tmp/runtime/work /tmp/runtime/merged >/dev/null 2>&1") != 0) {
        fprintf(stderr, "[Storage] Failed to create ephemeral directories.\n");
        return FALSE;
    }
    
    char options[1024];
    snprintf(options, sizeof(options), 
             "lowerdir=%s,upperdir=/tmp/runtime/upper,workdir=/tmp/runtime/work", 
             base_rootfs);

    // Mount the OverlayFS stack
    if (mount("overlay", "/tmp/runtime/merged", "overlay", 0, options) != 0) {
        perror("[Storage] OverlayFS mount failed");
        return FALSE;
    }

    strcpy(merged_path_out, "/tmp/runtime/merged");
    printf("[Storage] Ephemeral Copy-on-Write layer established.\n");
    return TRUE;
}

void clean_storage_host() {
    // Force cleanup of the temporary directories on the host side
    system("umount /tmp/runtime/merged >/dev/null 2>&1");
    system("rm -rf /tmp/runtime >/dev/null 2>&1");
}