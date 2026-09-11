#ifndef VERIFIER_H
#define VERIFIER_H

#include <stddef.h>
#include "typedefs.h"

/*
    Host directories that must never be used as a container's RootFS.
    Defined once in verifier.c; declared here so it isn't duplicated
    (and doesn't cause a multiple-definition link error) across translation
    units.
*/
extern const char *const insecure_paths[];
extern const size_t insecure_paths_count;

BOOL verify_config(int stack_size , const char* rootfs , const char* interactive_shell);
BOOL verify_stack_size(int stack_size);
BOOL verify_rootfs(const char *rootfs_path , const char* interactive_shell);
BOOL check_rootfs_security(const char *rootfs_path);
BOOL verify_visibility(const char *rootfs_path , const char* interactive_shell);
BOOL verify_procfs(void);

#endif