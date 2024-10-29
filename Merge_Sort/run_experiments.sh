#!/bin/bash

# Missing num_procs
missing_num_procs=(2 4 8 16 32 64 128 256 512 1024)

# Array sizes
array_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)

# Input types
input_types=("Sorted" "Random" "ReverseSorted" "1_perc_perturbed")

# Path to your job script
job_script="mpi.grace_job"

# Loop over the missing num_procs
for num_procs in "${missing_num_procs[@]}"; do
    for array_size in "${array_sizes[@]}"; do
        for input_type in "${input_types[@]}"; do
            # Submit the job
            sbatch $job_script $array_size $num_procs $input_type

            echo "Submitted job for num_procs=$num_procs, array_size=$array_size, input_type=$input_type"
        done
    done
done

# Submit the missing job for num_procs=128, array_size=1048576, input_type=1_perc_perturbed
num_procs=128
array_size=1048576
input_type="1_perc_perturbed"

# Submit the job
sbatch $job_script $array_size $num_procs $input_type

echo "Submitted job for num_procs=$num_procs, array_size=$array_size, input_type=$input_type"
