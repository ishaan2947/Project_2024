#!/bin/bash

# Array sizes
array_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)

for array_size in "${array_sizes[@]}"; do
    # Create a directory for the array size if it doesn't exist
    dir="a${array_size}"
    mkdir -p "$dir"

    # Move all Caliper files matching the array size into the directory
    mv p*-a${array_size}-t*.cali "$dir"/
done
