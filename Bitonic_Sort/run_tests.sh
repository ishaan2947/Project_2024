#!/bin/bash

# Usage: ./run_tests.sh <array_size>


if [ -z "$1" ]; then
    echo "Usage: $0 <array_size>"
    exit 1
fi

array_size=$1

# Input types
input_types=("sorted" "random" "reverse" "nearly_sorted")

# Number of processes
processes_list=(2 4 8 16 32 64 128 256 512 1024)

# Path to your job script template
job_script_template="mpi.grace_job"

for input_type in "${input_types[@]}"; do
    for processes in "${processes_list[@]}"; do

        # manually set __NODES__ and __MEM_PER_NODE__ in the job script beofre running

        
        sbatch $job_script_template $array_size $processes $input_type

        echo "Submitted job for array_size=$array_size, processes=$processes, input_type=$input_type"

    done
done
