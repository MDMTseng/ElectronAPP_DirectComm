#!/bin/bash

# Define directories
BACKEND_DIR="backend"

# Create build directory if it doesn't exist
if [ ! -d "$BACKEND_DIR"/build ]; then
  mkdir -p "$BACKEND_DIR"/build
fi

cd $BACKEND_DIR
cmake -S . -B build
cmake --build build
cd ..



# Define the distribution directory
DIST_DIR="dist"
rm -rf "$DIST_DIR"
# Create the distribution directory if it doesn't exist
if [ ! -d "$DIST_DIR"/backend ]; then
  mkdir -p "$DIST_DIR"/backend
fi

# Copy the build artifacts to the distribution directory
cp -r "$BACKEND_DIR"/build/libdlib.dylib "$DIST_DIR"/backend

echo "Build artifacts have been exported to the '$DIST_DIR' directory."


# Create the distribution directory if it doesn't exist
if [ ! -d "$DIST_DIR"/frontend ]; then
  mkdir -p "$DIST_DIR"/frontend
fi

# Copy the build artifacts to the distribution directory
cp -r frontend/* "$DIST_DIR"/frontend

echo "frontend artifacts have been exported to the '$DIST_DIR' directory."