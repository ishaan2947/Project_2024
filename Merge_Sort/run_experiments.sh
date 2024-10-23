#!/bin/bash

# Input sizes (2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28)
input_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)

# Input types
input_types=("Sorted" "Random" "ReverseSorted" "1_perc_perturbed")

# Number of processes
processes_list=(2 4 8 16 32 64 128 256 512 1024)

# Path to your job script template
job_script_template="mpi.grace_job"

for input_size in "${input_sizes[@]}"; do
    for input_type in "${input_types[@]}"; do
        for processes in "${processes_list[@]}"; do

            # Calculate the number of nodes required (assuming 32 cores per node)
            nodes=$(( (processes + 31) / 32 ))

            # Create a temporary job script
            job_script="job_${input_size}_${input_type}_${processes}.sh"
            cp $job_script_template $job_script

            # Replace placeholders with actual values
            sed -i "s/__NODES__/${nodes}/g" $job_script

            # Submit the job
            sbatch $job_script $input_size $processes $input_type

            echo "Submitted job for input_size=$input_size, input_type=$input_type, processes=$processes"

            # Remove the temporary job script if desired
            rm $job_script

        done
    done
done
