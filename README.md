# Container-Runtime — A Linux Container Runtime in C++

> A Docker-like container runtime built from scratch using Linux namespaces, cgroups v2, overlay filesystems, and seccomp — written in modern C++17.

---

## Table of Contents

- [Project Goal](#-project-goal)
- [How It Works](#-how-it-works)
- [Architecture](#-architecture)
- [Required Packages](#-required-packages)
- [Installation](#-installation)
- [Build Instructions](#-build-instructions)
- [Project Roadmap](#-project-roadmap)
- [References & Documentation](#-references--documentation)

---

## Project Goal

Most developers use Docker every day without understanding what happens under the hood.
This project demystifies containerization by reimplementing a working container runtime
from scratch — using only Linux kernel primitives and C++17.

**What this runtime does:**
- Runs an isolated process with its own PID, network, mount, UTS, IPC, and user namespace
- Gives the container its own root filesystem using overlay filesystems
- Limits container resources (CPU, memory, PIDs) using cgroups v2
- Filters dangerous system calls using seccomp BPF
- Provides a minimal CLI: `run`, `ps`, `exec`, `stop`, `rm`

**What this project is NOT:**
- A wrapper around Docker or containerd
- A virtual machine
- A cross-platform tool — it is Linux-only by design, just like Docker's core

---

## How It Works

When you run:
```bash
sudo ./mycontainer run ubuntu /bin/bash
```

The runtime performs the following steps in order:

```
1. Parse CLI arguments
2. Create new namespaces via clone() syscall
      PID  → container has its own process tree (PID 1)
      MNT  → container has its own mount points
      NET  → container has its own network stack
      UTS  → container has its own hostname
      IPC  → container has its own IPC resources
      USER → container has its own user/group IDs
3. Set up overlay filesystem
      lower layer  = read-only base image (e.g. Alpine Linux)
      upper layer  = writable container layer
      merged layer = what the container sees as its /
4. pivot_root into the merged overlay directory
5. Mount essential virtual filesystems
      /proc, /sys, /dev
6. Apply cgroups v2 resource limits
      memory.max, cpu.max, pids.max
7. Drop Linux capabilities (least privilege)
8. Apply seccomp BPF syscall filter
9. exec() the requested command inside the container
```

---

## Architecture

```
mycontainer/
├── src/
│   ├── main.cpp              # CLI entry point
│   ├── container/
│   │   ├── container.cpp     # Core container lifecycle
│   │   ├── container.hpp
│   │   ├── namespace.cpp     # Namespace setup (clone flags)
│   │   ├── namespace.hpp
│   │   ├── cgroup.cpp        # cgroups v2 resource limits
│   │   ├── cgroup.hpp
│   │   ├── filesystem.cpp    # overlayfs + pivot_root
│   │   ├── filesystem.hpp
│   │   ├── network.cpp       # veth pairs + NAT
│   │   ├── network.hpp
│   │   ├── seccomp.cpp       # syscall filtering
│   │   └── seccomp.hpp
│   ├── image/
│   │   ├── image.cpp         # Image layer management
│   │   └── image.hpp
│   └── cli/
│       ├── cli.cpp           # Command parser
│       └── cli.hpp
├── images/                   # Stored base image layers
├── containers/               # Active container state
├── tests/
│   ├── test_namespace.cpp
│   ├── test_cgroup.cpp
│   └── test_filesystem.cpp
├── docs/
│   ├── architecture.md
│   ├── namespaces.md
│   ├── cgroups.md
│   └── networking.md
├── CMakeLists.txt
├── .gitignore
└── README.md
```

---

## Required Packages

### Core Build Tools

| Package | Version | Purpose |
|---------|---------|---------|
| `g++` | ≥ 11.0 | C++17 compiler |
| `cmake` | ≥ 3.16 | Build system |
| `make` | any | Build runner |
| `git` | any | Version control |

### Linux Development Libraries

| Package | Purpose |
|---------|---------|
| `libseccomp-dev` | Seccomp BPF syscall filtering |
| `libcap-dev` | Linux capabilities manipulation |
| `libnetlink` / `libnl-3-dev` | Netlink socket for network setup |
| `liburing-dev` | (optional) io_uring for async I/O |

### Debugging & Tracing Tools

| Package | Purpose |
|---------|---------|
| `gdb` | C++ debugger |
| `strace` | Trace system calls — essential for this project |
| `ltrace` | Trace library calls |
| `valgrind` | Memory leak detection |
| `perf` | Performance profiling |

### Networking Tools

| Package | Purpose |
|---------|---------|
| `iproute2` | `ip` command — manage network namespaces, veth pairs |
| `iptables` | NAT rules for container internet access |
| `bridge-utils` | Network bridge management |

### Filesystem Tools

| Package | Purpose |
|---------|---------|
| `util-linux` | `mount`, `umount`, `unshare` commands |
| `e2fsprogs` | Filesystem utilities |
| `tar` | Extract base image rootfs |

### Testing

| Package | Purpose |
|---------|---------|
| `libgtest-dev` | Google Test framework |
| `cmake` | Already listed above |

---

## Installation

### Step 0 — Prerequisites

Make sure you are running on **Linux kernel 5.x or higher**.
If you are on Windows, use **WSL2** (Windows Subsystem for Linux 2):

```powershell
# Run in PowerShell as Administrator
wsl --install
# Restart your computer, then open Ubuntu from Start Menu
```

Verify your kernel version inside WSL2/Linux:
```bash
uname -r
# Should output something like: 5.15.90.1-microsoft-standard-WSL2
```

Verify cgroups v2 is active:
```bash
mount | grep cgroup2
# Should output a line containing: cgroup2
```

If cgroups v2 is not active, enable systemd in WSL2:
```bash
sudo nano /etc/wsl.conf
```
Add the following content:
```ini
[boot]
systemd=true
```
Then restart WSL2 from PowerShell:
```powershell
wsl --shutdown
wsl
```

---

### Step 1 — Update System

```bash
sudo apt update && sudo apt upgrade -y
```

---

### Step 2 — Install Core Build Tools

```bash
sudo apt install -y \
    build-essential \
    g++ \
    cmake \
    make \
    git
```

Verify installations:
```bash
g++ --version       # should show g++ 11.x or higher
cmake --version     # should show 3.16 or higher
git --version
```

---

### Step 3 — Install Linux Development Libraries

```bash
sudo apt install -y \
    libseccomp-dev \
    libcap-dev \
    libnl-3-dev \
    libnl-route-3-dev \
    pkg-config
```

---

### Step 4 — Install Debugging & Tracing Tools

```bash
sudo apt install -y \
    gdb \
    strace \
    ltrace \
    valgrind
```

---

### Step 5 — Install Networking Tools

```bash
sudo apt install -y \
    iproute2 \
    iptables \
    bridge-utils \
    net-tools
```

Enable IP forwarding (required for container networking):
```bash
echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
```

To make it permanent:
```bash
echo "net.ipv4.ip_forward=1" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

---

### Step 6 — Install Filesystem Tools

```bash
sudo apt install -y \
    util-linux \
    e2fsprogs \
    tar \
    wget
```

---

### Step 7 — Install Google Test

```bash
sudo apt install -y libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

---

### Step 8 — Download a Base Root Filesystem (Alpine Linux)

Your container needs a root filesystem to boot into.
Alpine Linux is only ~5MB — perfect for development:

```bash
mkdir -p ~/mycontainer/images/alpine
cd ~/mycontainer/images/alpine
wget https://dl-cdn.alpinelinux.org/alpine/v3.19/releases/x86_64/alpine-minirootfs-3.19.0-x86_64.tar.gz
tar xzf alpine-minirootfs-3.19.0-x86_64.tar.gz
rm alpine-minirootfs-3.19.0-x86_64.tar.gz
```

Verify it looks like a real Linux root:
```bash
ls ~/mycontainer/images/alpine
# Should show: bin  dev  etc  home  lib  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var
```

---

### Step 9 — Clone the Repository

---

## Build Instructions

```bash
# Inside the project directory
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

Run the container runtime:
```bash
sudo ./mycontainer run /bin/sh
```

Run tests:
```bash
cd build
ctest --verbose
```

---

## Project Roadmap

### Week 1 — Process & Namespace Isolation
- [ ] Set up CMake project structure
- [ ] Implement basic process spawning with `clone()`
- [ ] Add PID namespace isolation
- [ ] Add UTS namespace (custom hostname)
- [ ] Add IPC and MNT namespace
- [ ] Verify: `ps aux` inside container shows only 1 process

### Week 2 — Filesystem Isolation
- [ ] Implement `chroot` based isolation (simple version)
- [ ] Implement overlay filesystem setup
- [ ] Implement `pivot_root` to switch root filesystem
- [ ] Mount `/proc`, `/sys`, `/dev` inside container
- [ ] Verify: container sees Alpine filesystem, not host

### Week 3 — Resource Limits & Networking
- [ ] Implement cgroups v2 memory limit
- [ ] Implement cgroups v2 CPU limit
- [ ] Implement cgroups v2 PID limit
- [ ] Create `veth` pair for container networking
- [ ] Set up NAT so container can reach internet
- [ ] Assign container its own IP address
- [ ] Verify: container has internet access with limited resources

### Week 4 — Security & Polish
- [ ] Drop Linux capabilities inside container
- [ ] Implement seccomp BPF syscall filter
- [ ] Build full CLI: `run`, `ps`, `exec`, `stop`, `rm`
- [ ] Add image layer management
- [ ] Write unit tests for all components
- [ ] Write architecture documentation
- [ ] Record demo video
- [ ] Final GitHub polish

---

## References & Official Documentation

### Linux Kernel — Namespaces
- [namespaces(7) — Linux man page](https://man7.org/linux/man-pages/man7/namespaces.7.html)
- [clone(2) — Linux man page](https://man7.org/linux/man-pages/man2/clone.2.html)
- [unshare(2) — Linux man page](https://man7.org/linux/man-pages/man2/unshare.2.html)
- [pid_namespaces(7)](https://man7.org/linux/man-pages/man7/pid_namespaces.7.html)
- [network_namespaces(7)](https://man7.org/linux/man-pages/man7/network_namespaces.7.html)
- [mount_namespaces(7)](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html)
- [user_namespaces(7)](https://man7.org/linux/man-pages/man7/user_namespaces.7.html)

### Linux Kernel — cgroups
- [cgroups(7) — Linux man page](https://man7.org/linux/man-pages/man7/cgroups.7.html)
- [Kernel cgroups v2 documentation](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html)
- [cgroups v2 interface files reference](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#interface-files)

### Linux Kernel — Filesystems
- [overlayfs documentation](https://www.kernel.org/doc/html/latest/filesystems/overlayfs.html)
- [pivot_root(2) — Linux man page](https://man7.org/linux/man-pages/man2/pivot_root.2.html)
- [mount(2) — Linux man page](https://man7.org/linux/man-pages/man2/mount.2.html)

### Linux Kernel — Security
- [capabilities(7) — Linux man page](https://man7.org/linux/man-pages/man7/capabilities.7.html)
- [seccomp(2) — Linux man page](https://man7.org/linux/man-pages/man2/seccomp.2.html)
- [libseccomp documentation](https://github.com/seccomp/libseccomp)

### Networking
- [ip(8) — iproute2 man page](https://man7.org/linux/man-pages/man8/ip.8.html)
- [veth(4) — virtual ethernet devices](https://man7.org/linux/man-pages/man4/veth.4.html)
- [iptables documentation](https://www.netfilter.org/documentation/)

### Books
- **"Linux Kernel Development" — Robert Love** — Best intro to Linux kernel concepts
- **"Container Security" — Liz Rice** — Directly relevant to this project
- **"The Linux Programming Interface" — Michael Kerrisk** — Complete Linux systems programming reference

### Videos & Talks
- **"Containers from Scratch" — Liz Rice (GopherCon 2016)** — Watch this first. Same project concept in Go.
  [https://www.youtube.com/watch?v=8fi7uSYlOdc](https://www.youtube.com/watch?v=8fi7uSYlOdc)
- **"Container Runtimes" — Kristen Jacobs (KubeCon)** — How container runtimes fit into Kubernetes
- **"Linux Namespaces" — Michael Kerrisk (Linux.conf.au)** — Deep dive into namespaces

### Source Code References
- [runc — OCI container runtime in Go](https://github.com/opencontainers/runc) — The real production runtime Docker uses
- [bocker — Docker in ~100 lines of bash](https://github.com/p8952/bocker) — Great for understanding the concept simply
- [Linux kernel source — namespace implementation](https://github.com/torvalds/linux/tree/master/kernel)

### Standards & Specifications
- [OCI Runtime Specification](https://github.com/opencontainers/runtime-spec) — The standard your runtime loosely follows
- [OCI Image Format Specification](https://github.com/opencontainers/image-spec)

---

## System Requirements

| Requirement | Minimum |
|-------------|---------|
| OS | Linux (kernel 5.x+) or WSL2 on Windows |
| Architecture | x86_64 |
| RAM | 512 MB free |
| Disk | 1 GB free |
| Compiler | g++ 11+ or clang++ 13+ |
| CMake | 3.16+ |
| Privileges | Must run as root (`sudo`) |

---

## Important Notes

- **Must run as root:** namespace and cgroup operations require root privileges.
- **WSL2 users:** all features work on WSL2 with kernel 5.15+. Run `uname -r` to verify.
- **cgroups v2 only:** this runtime targets cgroups v2. cgroups v1 is not supported.
- **x86_64 only:** the Alpine rootfs downloaded above is for x86_64 architecture.
