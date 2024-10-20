# CSCE 435 Group project

## 0. Group number: 21

## 1. Group members:
1. Sathvik Yeruva
2. Ishaan Nigam
3. Yusa Sagli
4. Mustafa Tekin
   
Note: We will communicate through imessage on our phones  
## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

Bitonic Sort(Ishaan):This is a comparison-based sorting algorithm that is well-suited for parallel computing. It works by recursively sorting a sequence into a
bitonic sequence and then merging it.This algorithm is ment to be highly efficient on parallel architectures due to its regular structure.

Bitonic Sort Implementation Updates and Questions(Ishaan):
- Implementing the parallel Bitonic Sort algorithm has been challenging, particularly in ensuring that data is correctly sorted across all processes.
- Can you verify if my implementation is correct?
- Implementation Description: In my implementation of the parallel bitonic sort algorithm, the input data is divided among multiple processes, each performing a local bitonic sort. At each stage, processes exchange data with a partner (determined by XORing their rank with a mask) and perform compare-exchange operations, retaining smaller or larger elements depending on the sorting direction. I used recursive merging to ensure local data is sorted before communication between processes. Once all stages are complete, the root process gathers the globally sorted data using MPI_Gather.
    
Sample Sort(Mustafa):This is a divide-and-conquer sorting algorithm that is well-suited for parallel computation. This algorithm wokrs by picking a set of sample from the input using it to partition the data into smaller buckets and sorting each bucket individually. Sample sort is designed in such a way to minimize inter-process communication, making it highly scalable.
  
Merge Sort(Sathvik): Merge Sort is a divide-and-conquer sorting algorithm that functions by recursively splitting the data set into smaller sublists, sorts them and then merges them together to create a fully sorted list. Merge Sort can use parallel architectures highly efficiently as the portions of data that the original data set is broken can be distributed across several processors which would allow for simultaneous sorting and merging of the sublists.
  
Radix Sort(Yusa): Radix Sort is a non-comparison-based sorting algorithm. It works by sorting numbers digit by digit, starting from the least significant digit to the most significant one. It's especially efficient for parallel computing because each digit can be sorted independently using counting sort or a similar linear-time algorithm. In this project, Radix Sort will be implemented and parallelized using MPI.

Architectures and Tools:
- Architecture: Distributed-memory systems using MPI on the Grace supercomputing platform.
- Parallelization Strategies: Implementing both master/worker and SPMD models.
- Performance Measurement: Using Caliper for performance instrumentation and Thicket for analysis.
  

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

Bitonic Sort (Ishaan- UPDATED):

```text
#### Bitonic Sort (Ishaan- UPDATED):

Initialize MPI environment
    - MPI_Init to set up parallel processing
    - Determine rank (process ID) and size (number of processes)

Generate and distribute data:
    - Rank 0 generates the global data (random, sorted, reverse, or nearly sorted)
    - Scatter data to all processes using MPI_Scatter

Perform local bitonic sort on the local data:
    - Recursively sort local data into a bitonic sequence
    - Use compare-exchange operations to merge sequences

for k = 0 to log2(size) - 1 do // Stages
    for j = k down to 0 do     // Steps within stages
        mask = 1 << j
        partner = rank XOR mask

        Determine sorting direction:
            if ((rank >> (k + 1)) & 1) == 0 then
                dir = ASCENDING
            else
                dir = DESCENDING
            end if

        Exchange data with partner:
            - Use MPI_Sendrecv to exchange local data with the partner process

        Perform compare-exchange with received data:
            if rank < partner then
                if dir == ASCENDING then
                    Compare and keep lower elements in the local array
                else
                    Compare and keep higher elements in the local array
                end if
            else
                if dir == ASCENDING then
                    Compare and keep higher elements in the local array
                else
                    Compare and keep lower elements in the local array
                end if
            end if

    end for
end for

Synchronize all processes using MPI_Barrier

Gather sorted data at the root process using MPI_Gather

Print execution time at root

Finalize MPI environment using MPI_Finalize


```
Sample Sort (Mustafa):

```text
#### Sample Sort (Mustafa - UPDATED):

    Initialize MPI environment
Determine rank (process ID) and size (number of processes)
Generate local portion of data

if rank == 0 then
    Collect samples from all processes:
        for each process i > 0:
            Receive local samples using MPI_Recv from process i
    Sort the collected samples
else
    Send local samples to root using MPI_Send

if rank == 0 then
    Broadcast sorted samples to all processes using MPI_Send:
        for each process i > 0:
            Send sorted samples to process i
else
    Receive sorted samples from root using MPI_Recv

if rank == 0 then
    Choose splitters from sorted samples
    Broadcast(Communicate) splitters to all processes using MPI_Send:
        for each process i > 0:
            Send splitters to process i
else
    Receive splitters from root using MPI_Recv


Partition local data into buckets based on splitters

Exchange buckets between processes using MPI_Send and MPI_Recv:
    for each process i:
        Send relevant bucket to process i using MPI_Send
        Receive bucket from process i using MPI_Recv


Perform local sort on the received bucket


Gather sorted buckets at the root using MPI_Send and MPI_Recv:
    if rank == 0:
        for each process i > 0:
            Receive sorted bucket from process i using MPI_Recv
        Merge the sorted buckets into the final sorted array
    else
        Send sorted bucket to root using MPI_Send

Finalize MPI environment

```

Merge Sort(Sathvik - UPDATED):
```text
#### Merge Sort(Sathvik - UPDATED):
    Initialize MPI environment
    Determine rank (process ID) and size (number of processes)

    If rank == 0:
        Generate/Read Full Data Set based on input_type
        Divide the data set into equal chunks
        Distribute chunks to all processors including itself with MPI_Scatterv
        Print a sample of the initial data
    Else:
        Receive chunk of data using MPI_Scatterv

    Perform local sort on received data (e.g., Quicksort)

    active = True
    step = 1
    While step < size:
        If active:
            If rank % (2 * step) == 0:
                If rank + step < size then:
                    Exchange sizes with processor(rank + step) using MPI_Sendrecv
                    Receive sorted data from processor(rank + step) with MPI_Recv
                    Merge this data with local data to make a larger sorted list
            Else if rank % (2 * step) == step:
                Send size to processor(rank - step) using MPI_Sendrecv
                Send sorted local data to processor(rank - step) with MPI_Send
                active = False  // Process becomes inactive but stays in the loop
        Synchronize all processes with MPI_Barrier
        step = step * 2

    If rank == 0:
        Verify that data is correctly sorted
        Output or store the fully sorted data in local data
        Print a sample of the sorted data

    Finalize MPI environment
```

Radix Sort (Yusa - Updated):

```text
#### Radix Sort (Yusa):

    Initialize MPI environment
Determine rank (process ID) and size (number of processes)

If rank == 0 then
    Generate/Read full data set 'data' based on 'input_type'
Else
    data remains uninitialized (NULL)

Ensure total number of elements 'n' is divisible by 'size'
If not divisible then
    If rank == 0 then
        Output error message: "Array size must be divisible by number of processes."
    Finalize MPI environment
    Exit program

Calculate local number of elements 'local_n' as 'n / size'
Allocate memory for 'local_data' with size 'local_n'

If rank == 0 then
    // Data Initialization Region (Caliper annotated as 'data_init_runtime')
    Initialize 'data' based on 'input_type' (e.g., random, sorted, reverse)

Distribute data to all processes using MPI_Scatter:
    Each process receives 'local_n' elements into 'local_data' from 'data'

Compute local maximum value 'local_max' from 'local_data'

Compute global maximum value 'global_max' using MPI_Allreduce with MPI_MAX on 'local_max'

Synchronize all processes using MPI_Barrier
Record 'start_time' using MPI_Wtime()

for each digit position 'exp' (starting from 1, while 'global_max / exp > 0') do:

    Perform local counting sort on 'local_data' based on current digit 'exp'

    Gather all sorted subarrays at root process using MPI_Gather:
        Root process collects sorted 'local_data' from all processes into 'gathered_data'

    If rank == 0 then
        Perform counting sort on 'gathered_data' based on current digit 'exp'

    Scatter the globally sorted data back to all processes using MPI_Scatter:
        Each process updates 'local_data' with its portion of 'gathered_data'

    If rank == 0 then
        Free memory allocated for 'gathered_data'

end for

Synchronize all processes using MPI_Barrier
Record 'end_time' using MPI_Wtime()

Gather final sorted data at root process using MPI_Gather:
    Root process collects 'local_data' from all processes into 'data'

If rank == 0 then
    Output total time taken: 'end_time - start_time'
    Optionally, verify correctness of 'data'
    Free memory allocated for 'data'

Free memory allocated for 'local_data'

Finalize MPI environment

```

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)

Metrics to Measure:
- Execution Time: Total time taken by each algorithm to sort the data.
- Speedup: How much faster the parallel algorithm is compared to the sequential version.
- Efficiency: Ratio of speedup to the number of processors.
- Scalability: How performance changes with varying numbers of processors (strong and weak scaling).
- Communication Overhead: Time spent in communication between processors.
  
Input Sizes:
- 1024, 2048, 4096, 8192, 16384
  
Input Types:
- Sorted: Data already in order.
- Sorted with 1% Perturbed: Nearly sorted data with minor perturbations.
- Random: Data in random order.
- Reverse Sorted: Data sorted in reverse order.

Scaling Experiments:

   1.) Strong Scaling: 
      - Keeping the problem size constant while increasing the number of processors.By doing this we can observe how the execution time decreases as more      
        processors are added.
      - Plan:Choose a fixed large dataset.Run each algorithm on varying numbers of processors (e.g., 2, 4, 8, 16, 32).Record execution times and calculate speedup          and efficiency.
   
   2.) Weak Scaling:
      - Increasing problem size while also increasing the number of processors. The problem size proportion should remain constant with for the number of
      processors (proportional).
      - Plan: Each processor should have 1000 values to sort through, so 2 processors would have 512 values while 32 processors
      would have 16384 values to sort. Keeping the problem size proportially constant to the number of processes.

### 3a. Caliper instrumentation
Please use the caliper build `/scratch/group/csce435-f24/Caliper/caliper/share/cmake/caliper` 
(same as lab2 build.sh) to collect caliper files for each experiment you run.

Your Caliper annotations should result in the following calltree
(use `Thicket.tree()` to see the calltree):
```
main
|_ data_init_X      # X = runtime OR io
|_ comm
|    |_ comm_small
|    |_ comm_large
|_ comp
|    |_ comp_small
|    |_ comp_large
|_ correctness_check
```

Required region annotations:
- `main` - top-level main function.
    - `data_init_X` - the function where input data is generated or read in from file. Use *data_init_runtime* if you are generating the data during the program, and *data_init_io* if you are reading the data from a file.
    - `correctness_check` - function for checking the correctness of the algorithm output (e.g., checking if the resulting data is sorted).
    - `comm` - All communication-related functions in your algorithm should be nested under the `comm` region.
      - Inside the `comm` region, you should create regions to indicate how much data you are communicating (i.e., `comm_small` if you are sending or broadcasting a few values, `comm_large` if you are sending all of your local values).
      - Notice that auxillary functions like MPI_init are not under here.
    - `comp` - All computation functions within your algorithm should be nested under the `comp` region.
      - Inside the `comp` region, you should create regions to indicate how much data you are computing on (i.e., `comp_small` if you are sorting a few values like the splitters, `comp_large` if you are sorting values in the array).
      - Notice that auxillary functions like data_init are not under here.
    - `MPI_X` - You will also see MPI regions in the calltree if using the appropriate MPI profiling configuration (see **Builds/**). Examples shown below.

All functions will be called from `main` and most will be grouped under either `comm` or `comp` regions, representing communication and computation, respectively. You should be timing as many significant functions in your code as possible. **Do not** time print statements or other insignificant operations that may skew the performance measurements.

### **Nesting Code Regions Example** - all computation code regions should be nested in the "comp" parent code region as following:
```
CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_small");
sort_pivots(pivot_arr);
CALI_MARK_END("comp_small");
CALI_MARK_END("comp");

# Other non-computation code
...

CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_large");
sort_values(arr);
CALI_MARK_END("comp_large");
CALI_MARK_END("comp");
```

### **Calltree Example**:
```
# MPI Mergesort
4.695 main
├─ 0.001 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 2.599 comm
│  ├─ 2.572 MPI_Barrier
│  └─ 0.027 comm_large
│     ├─ 0.011 MPI_Gather
│     └─ 0.016 MPI_Scatter
├─ 0.910 comp
│  └─ 0.909 comp_large
├─ 0.201 data_init_runtime
└─ 0.440 correctness_check
```

### 3b. Collect Metadata

Have the following code in your programs to collect metadata:
```
adiak::init(NULL);
adiak::launchdate();    // launch date of the job
adiak::libraries();     // Libraries used
adiak::cmdline();       // Command line used to launch the job
adiak::clustername();   // Name of the cluster
adiak::value("algorithm", algorithm); // The name of the algorithm you are using (e.g., "merge", "bitonic")
adiak::value("programming_model", programming_model); // e.g. "mpi"
adiak::value("data_type", data_type); // The datatype of input elements (e.g., double, int, float)
adiak::value("size_of_data_type", size_of_data_type); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
adiak::value("input_type", input_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
adiak::value("scalability", scalability); // The scalability of your algorithm. choices: ("strong", "weak")
adiak::value("group_num", group_number); // The number of your group (integer, e.g., 1, 10)
adiak::value("implementation_source", implementation_source); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
```

They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.
## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance. 
Include figures and explanation of your analysis.

### 4a. Vary the following parameters
For input_size's:
- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:
- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:
- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script 
that will run a for loop over the parameters above (e.g., on 64 processors, 
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).  

### 4c. You should measure the following performance metrics
- `Time`
    - Min time/rank
    - Max time/rank
    - Avg time/rank
    - Total time
    - Variance time/rank


## 5. Presentation
Plots for the presentation should be as follows:
- For each implementation:
    - For each of comp_large, comm, and main:
        - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
        - Strong scaling speedup plot for each input_type (4 plots)
        - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
