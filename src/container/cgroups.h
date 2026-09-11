#ifndef CGROUPS_H
#define CGROUPS_H

#include "typedefs.h"

#define CGROUP_BASE_PATH "/sys/fs/cgroup"
#define CGROUP_PERMISSION 0755

/*
    Create a cgroup for the container, apply memory limits, 
    and attach the target PID to it.
*/
BOOL setup_cgroups(const char *container_name, int child_pid, long memory_limit_bytes);

/*
    Remove the cgroup directory after the container exits.
*/
BOOL clean_cgroups(const char *container_name);

#endif // CGROUPS_H