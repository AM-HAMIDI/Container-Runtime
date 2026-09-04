cmake_minimum_required(VERSION 3.10)
project(ContainerRuntime_C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =========== External Packages to use ===========
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED json-c)

# =========== Add executable ===========
add_executable(${PROJECT_NAME}
    src/main.c 
    src/config_handler.c
)

# =========== Definitions ===========
set(DEFAULT_CONFIG_FILE "config.json" CACHE STRING "Default container config file name")
set(DEFAULT_CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/${DEFAULT_CONFIG_FILE}" CACHE STRING "Default path to the container config file")
set(DEFAULT_CONTAINER_NAME "default_container" CACHE STRING "Default hostname if not provided")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    DEFAULT_CONFIG_PATH="${DEFAULT_CONFIG_PATH}"
    DEFAULT_CONTAINER_NAME="${DEFAULT_CONTAINER_NAME}"
)

# =========== Link libraries ===========
target_include_directories(${PROJECT_NAME} PRIVATE ${JSONC_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${JSONC_LIBRARIES})