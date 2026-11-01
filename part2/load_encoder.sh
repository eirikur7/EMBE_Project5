#!/bin/bash
# Loads the encoder kernel module and makes /dev/encoder (like in assignment 5.3).
# Run: sudo ./load_encoder.sh

# Load the module
insmod lkm/encoder.ko

# Find the major number the kernel gave it
MAJOR_NUMBER=$(awk '$2 == "encoder" {print $1}' /proc/devices)
echo "encoder major number: $MAJOR_NUMBER"

# Make the device node, everyone can read it
rm -f /dev/encoder
mknod -m 666 /dev/encoder c $MAJOR_NUMBER 0

ls -l /dev/encoder
