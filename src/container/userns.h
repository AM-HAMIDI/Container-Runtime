#ifndef USERNS_H
#define USERNS_H

#include "typedefs.h"

/*
    Maps the container's root user (UID 0) to the host's normal user.
*/
BOOL setup_user_mapping(int child_pid, const char *rootfs_path);

#endif // USERNS_H