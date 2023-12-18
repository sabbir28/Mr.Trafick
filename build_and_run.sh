#!/bin/bash

# Check if cmake and make are installed
if ! command -v cmake &> /dev/null || ! command -v make &> /dev/null; then
    echo "Error: cmake and make are required. Please install them first."
    exit 1
fi

# Create a build directory if it doesn't exist
mkdir -p build

# Navigate to the build directory
cd build || exit

# Run cmake to generate build files
cmake ..

# Build the project
make

# Check if the executable exists before running it
if [ -f HTTPSniffer ]; then
    # Run the executable
    ./HTTPSniffer
else
    echo "Error: Executable not found. Build may have failed."
fi

# Return to the original directory
cd ..
