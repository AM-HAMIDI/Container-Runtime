# =========== Project setup ===========
cmake_minimum_required(VERSION 3.12)
project(ContainerRuntime_C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(PROJECT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# =========== External Packages to use ===========
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED json-c)

# =========== Collect all source files ===========
file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

# =========== Add executable ===========
add_executable(${PROJECT_NAME} ${SRC})

# =========== Definitions ===========
set(DEFAULT_LINUX_DISTRIBUTION "alpine" CACHE STRING "Default linux distribution")
set(DEFAULT_CONFIG_FILE "config.json" CACHE STRING "Default container config file name")
set(DEFAULT_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/${DEFAULT_CONFIG_FILE}" CACHE STRING "Default path to the container config file")
set(DEFAULT_ROOTFS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/rootfs/${DEFAULT_LINUX_DISTRIBUTION}/" CACHE STRING "Default path to rootfs")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    DEFAULT_CONFIG_PATH="${DEFAULT_CONFIG_PATH}"
    DEFAULT_ROOTFS_PATH="${DEFAULT_ROOTFS_PATH}"
)

# =========== Include Directories ===========
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SRC_DIR}
    ${PROJECT_SRC_DIR}/cli
    ${PROJECT_SRC_DIR}/container
    ${PROJECT_SRC_DIR}/debug
    ${PROJECT_SRC_DIR}/verifier
    ${JSONC_INCLUDE_DIRS}
)

# =========== Link libraries ===========
target_link_libraries(${PROJECT_NAME} PRIVATE ${JSONC_LIBRARIES})