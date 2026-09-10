#!/bin/bash

# 1. Define the absolute project path
PROJECT_DIRECTORY="/home/amirmohammad-hamidi/Desktop/PROJECTS/Container_Runtime"

# 2. Navigate to the project root immediately, or exit if it doesn't exist
cd "$PROJECT_DIRECTORY" || { echo "Error: Project directory not found"; exit 1; }

# 3. Safely export variables from the .env file (checking if it exists first)
if [ -f ".env" ]; then
    export $(grep -v '^#' .env | xargs)
else
    echo "Warning: .env file not found. Proceeding without it."
fi

# 4. Clean the old build environment
# (Since we are already in the project root, we can safely use relative paths)
rm -rf build
mkdir build

# 5. Enter the build directory and compile, aborting if the cd fails
cd build || exit 1
cmake ..
make

sudo ./ContainerRuntime_C --default-config
