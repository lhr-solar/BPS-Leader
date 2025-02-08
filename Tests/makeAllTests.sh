#!/bin/bash

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Makefile is in the parent directory
MAKEFILE_DIR="$(dirname "$SCRIPT_DIR")"

echo "Testing production"
make "$MAKEFILE_DIR" -j
if [ $? -eq 0 ]; then
        echo "✅ Compilation successful for production code"
else
        echo "❌ Compilation failed for production code"
        exit 1
fi

# Find all .c files in the script's directory
C_FILES=("$SCRIPT_DIR"/*.c)

# Loop through each C file
for FILE in "${C_FILES[@]}"; do
    TEST_NAME=$(basename "$FILE" .c)
    
    echo "Compiling: $TEST_NAME.c"

    make "$MAKEFILE_DIR" TEST="$TEST_NAME" -j

    if [ $? -eq 0 ]; then
        echo "✅ Compilation successful for $TEST_NAME.c"
    else
        echo "❌ Compilation failed for $TEST_NAME.c"
        exit 1
    fi

    echo "Cleaning up..."
    make "$MAKEFILE_DIR" clean
done

echo "✅ All tests successful!"
