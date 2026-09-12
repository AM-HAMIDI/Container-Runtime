#include "network.h"
#include <stdio.h>
#include <stdlib.h>

BOOL setup_network_host(int child_pid) {
    char cmd[512];
    
    // 1. Create interconnected veth pair
    if (system("ip link add veth0 type veth peer name veth1 >/dev/null 2>&1") != 0) return FALSE;
    
    // 2. Move veth1 into the container's namespace
    snprintf(cmd, sizeof(cmd), "ip link set veth1 netns %d >/dev/null 2>&1", child_pid);
    if (system(cmd) != 0) return FALSE;

    // 3. Configure host side
    if (system("ip addr add 10.0.0.1/24 dev veth0 >/dev/null 2>&1") != 0) return FALSE;
    if (system("ip link set veth0 up >/dev/null 2>&1") != 0) return FALSE;

    // 4. Enable NAT so the container can access the external internet
    system("echo 1 > /proc/sys/net/ipv4/ip_forward");
    system("iptables -t nat -A POSTROUTING -s 10.0.0.0/24 -j MASQUERADE >/dev/null 2>&1");

    return TRUE;
}

BOOL setup_network_container() {
    // Configure container side loopback and veth1
    if (system("ip link set lo up >/dev/null 2>&1") != 0) return FALSE;
    if (system("ip addr add 10.0.0.2/24 dev veth1 >/dev/null 2>&1") != 0) return FALSE;
    if (system("ip link set veth1 up >/dev/null 2>&1") != 0) return FALSE;
    
    // Route traffic through the host cable
    if (system("ip route add default via 10.0.0.1 >/dev/null 2>&1") != 0) return FALSE;

    return TRUE;
}

void clean_network_host() {
    system("ip link del veth0 >/dev/null 2>&1");
    system("iptables -t nat -D POSTROUTING -s 10.0.0.0/24 -j MASQUERADE >/dev/null 2>&1");
}