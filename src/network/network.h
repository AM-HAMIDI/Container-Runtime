#ifndef NETWORK_H
#define NETWORK_H

#include "typedefs.h"

BOOL setup_network_host(int child_pid);
BOOL setup_network_container();
void clean_network_host();

#endif