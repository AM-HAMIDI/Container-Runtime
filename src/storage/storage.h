#ifndef STORAGE_H
#define STORAGE_H

#include "typedefs.h"

BOOL setup_overlayfs(const char *base_rootfs, char *merged_path_out);
void clean_storage_host();

#endif