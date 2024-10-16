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
#### Sample Sort (Mustafa):

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

Radix Sort (Yusa):

```text
#### Radix Sort (Yusa):

    Initialize MPI environment
    Determine rank (process ID) and size (number of processes)
    
    If rank == 0 then
        Generate/Read Full Data Set
        Determine the maximum value in the dataset
        Broadcast maximum value to all processes using MPI_Bcast
    Else:
        Receive the maximum value using MPI_Bcast
    
    Calculate the number of digits needed to represent the maximum value (for radix sorting)

    for each digit (from least significant to most significant) do:

        Perform local counting sort on the current digit:
            Create a count array of size 10 (for base-10 digits)
            for each element in local data do:
                Extract the current digit
                Increment the corresponding count in the count array
        
        Use MPI_Allreduce to combine local counts into global counts
        Broadcast the global count to all processes using MPI_Allreduce

        Calculate prefix sums across all processes:
            Use MPI_Scan to calculate the prefix sum to determine the starting index for each process

        Redistribute data based on the sorted order of the current digit:
            for each process do:
                Send sorted data chunks to the corresponding process using MPI_Send
                Receive sorted data chunks from other processes using MPI_Recv

        After redistribution, each process has sorted data based on the current digit

    end for

    If rank == 0 then
        Gather the fully sorted data from all processes using MPI_Gather
        Output the sorted data
    Else:
        Send the sorted local data to the root process using MPI_Gather

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
