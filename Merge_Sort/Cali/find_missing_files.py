#!/usr/bin/env python3

import os

# Define the expected parameters
num_procs_list = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
array_sizes = [65536, 262144, 1048576, 4194304, 16777216, 67108864, 268435456]
input_types = ["Sorted", "Random", "ReverseSorted", "1_perc_perturbed"]

# Directory prefix for array size folders
array_size_prefix = "a"

# Initialize a list to store missing files
missing_files = []

# Loop over each combination of parameters
for array_size in array_sizes:
    # Define the directory for the current array size
    directory = f"{array_size_prefix}{array_size}"
    
    # Check if the directory exists
    if not os.path.isdir(directory):
        print(f"Directory '{directory}' does not exist.")
        continue
    
    # Loop over num_procs and input_types
    for num_procs in num_procs_list:
        for input_type in input_types:
            # Construct the expected filename
            filename = f"p{num_procs}-a{array_size}-t{input_type}.cali"
            # Construct the full path to the file
            filepath = os.path.join(directory, filename)
            # Check if the file exists
            if not os.path.isfile(filepath):
                # If not, add it to the missing files list
                missing_files.append(filepath)

# Print the missing files
if missing_files:
    print("Missing Caliper files:")
    for file in missing_files:
        print(file)
else:
    print("All expected Caliper files are present.")
