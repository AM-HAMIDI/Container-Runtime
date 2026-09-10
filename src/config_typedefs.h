#pragma once

// Config files fields
#define FILED_HOSTNAME "hostname"
#define FILED_ROOTFS_PATH "rootfs_path"
#define FILED_INTERACTIVE_SHELL "interactive_shell"
#define FILED_STACK_SIZE "stack_size"

// default configs
#define DEFAULT_HOSTNAME "default-container"
#define DEFAULT_INTERACTIVE_SHELL "/bin/sh"

#define MIN_STACK_SIZE (64 * 1024)        /* 64 KB */
#define MAX_STACK_SIZE (64 * 1024 * 1024) /* 64 MB */
#define DEFAULT_STACK_SIZE (1024 * 1024)  /* 1 MB*/