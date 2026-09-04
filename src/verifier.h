#ifndef VERIFIER_H
#define VERIFIER_H

const char *insecure_path =
    {};

int verify_rootfs(const char *rootfs_path);
int check_rootfs_security(const char *rootfs_path);
int verify_visibility(const char *rootfs_path);

#endif