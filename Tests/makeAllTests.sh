#!/bin/bash

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Go one directory up to find the Makefile
MAKEFILE_DIR="$(dirname "$SCRIPT_DIR")"

# Find all .c files in the script's directory
C_FILES=("$SCRIPT_DIR"/*.c)

# Loop through each C file
for FILE in "${C_FILES[@]}"; do
    # Extract filename without path and extension
    TEST_NAME=$(basename "$FILE" .c)
    
    echo "Compiling: $TEST_NAME.c"

    # Run the make command
    make -C "$MAKEFILE_DIR" TEST="$TEST_NAME"

    # Check if make was successful
    if [ $? -eq 0 ]; then
        echo "✅ Compilation successful for $TEST_NAME.c"
    else
        echo "❌ Compilation failed for $TEST_NAME.c"
        exit 1
    fi

    # Run make clean
    echo "Cleaning up..."
    make -C "$MAKEFILE_DIR" clean
done

echo "✅ All tests successful!"
