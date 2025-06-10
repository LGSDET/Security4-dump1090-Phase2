#!/bin/sh

BIN_DIR=/home/lg/dump1090/

# Create BIN_DIR if it doesn't exist
if [ ! -d "$BIN_DIR" ]; then
    echo "Creating directory: $BIN_DIR"
    mkdir -p "$BIN_DIR"
fi

# Copy binaries to BIN_DIR
cp -p dump1090 "$BIN_DIR"
cp -p sqlog_viewer "$BIN_DIR"

echo "Installation completed to $BIN_DIR"
