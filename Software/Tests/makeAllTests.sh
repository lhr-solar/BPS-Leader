#!/bin/bash

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Makefile is in the parent directory
MAKEFILE_DIR="$(dirname "$SCRIPT_DIR")"

make -C "$MAKEFILE_DIR" clean -j8

echo "Testing production"
make "$MAKEFILE_DIR" -j8
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
    # Removes the .c extension first
    NAME=$(basename "$FILE" .c)

    # Removes "_test" from the end of the string
    TEST_NAME="${NAME%_test}"
    
    echo "Compiling: $NAME.c"

    make -C "$MAKEFILE_DIR" TEST="$NAME" -j8

    if [ $? -eq 0 ]; then
        echo "✅ Compilation successful for $NAME.c"
    else
        echo "❌ Compilation failed for $NAME.c"
        exit 1
    fi

    echo "Cleaning up..."
    make -C "$MAKEFILE_DIR" clean
done

echo "✅ All tests successful!"