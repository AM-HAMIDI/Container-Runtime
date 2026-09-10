cmake_minimum_required(VERSION 3.10)
project(ContainerRuntime_C)

# Generate compile_commands.json for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =========== External Packages to use ===========
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED json-c)

# =========== Add executable ===========
# All implementation files must be listed here to link properly
add_executable(${PROJECT_NAME}
    src/main.c 
    src/cli.c
    src/config_manager.c
    src/verifier.c
    src/cgroups.c
)

# =========== Definitions ===========
set(DEFAULT_LINUX_DISTRIBUTION "alpine" CACHE STRING "Default linux distribution")
set(DEFAULT_CONFIG_FILE "config.json" CACHE STRING "Default container config file name")
set(DEFAULT_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/${DEFAULT_CONFIG_FILE}" CACHE STRING "Default path to the container config file")
set(DEFAULT_ROOTFS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/rootfs/${DEFAULT_LINUX_DISTRIBUTION}/" CACHE STRING "Default path to rootfs")

# Pass CMake variables to C code as macros
target_compile_definitions(${PROJECT_NAME} PRIVATE
    DEFAULT_CONFIG_PATH="${DEFAULT_CONFIG_PATH}"
    DEFAULT_ROOTFS_PATH="${DEFAULT_ROOTFS_PATH}"
)

# =========== Link libraries ===========
target_include_directories(${PROJECT_NAME} PRIVATE ${JSONC_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${JSONC_LIBRARIES})