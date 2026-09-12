#include "seccomp_filter.h"
#include <stdio.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <errno.h>
#include <sys/syscall.h>

BOOL setup_seccomp() {
    struct sock_filter filter[] = {
        // 1. Load the architecture identity
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, arch))),
        
        // 2. Kill the process if it is not running on standard x86_64
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),

        // 3. Load the requested system call number
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),

        // 4. Check if the syscall is 'chmod' or 'fchmodat' (used by modern tools)
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_chmod, 2, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_fchmodat, 1, 0),

        // 5. If it does not match the blocked calls, ALLOW it
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        // 6. If it matches a blocked call, return EPERM (Operation not permitted)
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | (EPERM & SECCOMP_RET_DATA))
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    // You MUST prevent the process from gaining new privileges before applying Seccomp
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
        perror("[Seccomp] prctl(PR_SET_NO_NEW_PRIVS) failed");
        return FALSE;
    }

    // Attach the BPF program to the kernel
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0) {
        perror("[Seccomp] prctl(PR_SET_SECCOMP) failed");
        return FALSE;
    }

    printf("[Seccomp] Syscall filtering activated.\n");
    return TRUE;
}