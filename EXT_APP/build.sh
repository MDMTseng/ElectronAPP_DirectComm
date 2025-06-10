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