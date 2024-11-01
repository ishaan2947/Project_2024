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


Bitonic Sort (Ishaan) Call Tree:
 
```text 
 
Path                 Min time/rank Max time/rank Avg time/rank Time %     
main                      0.000080      0.000098      0.000089  0.677316  
  comm                    0.000024      0.000026      0.000025  0.188056  
    comm_large            0.000029      0.000038      0.000033  0.254078  
      MPI_Scatter         0.003863      0.004273      0.004068 30.995997  
      MPI_Gather          0.001040      0.001048      0.001044  7.955633  
  data_init_runtime       0.000039      0.000039      0.000039  0.149396  
  MPI_Barrier             0.002429      0.002772      0.002600 19.812123  
  comp                    0.000015      0.000016      0.000015  0.116183  
    comp_large            0.000204      0.000227      0.000215  1.641660  
  mpi_bitonic_sort        0.000023      0.000025      0.000024  0.180948  
    comm                  0.000013      0.000014      0.000013  0.100293  
      comm_large          0.000018      0.000021      0.000020  0.150093  
        MPI_Sendrecv      0.001326      0.001389      0.001357 10.342060  
    comp_small            0.000011      0.000011      0.000011  0.085211  
  MPI_Comm_dup            0.003230      0.003265      0.003247 24.741450  

```

### Performance Evaluation

#### 1. Bitonic Sort (Ishaan)

**Graphs:**  

- **Main:**
  
  - Strong Scaling Plots for Each Input Size:    
    ![image](https://github.com/user-attachments/assets/90f9be74-3376-4277-a7c4-c4f1c612f17c)
    ![image](https://github.com/user-attachments/assets/fc34298e-f325-4975-9e0b-4ea61f632ff5)
    ![image](https://github.com/user-attachments/assets/2c86e321-bd21-48b7-99a3-f0a8ecf38813)
    ![image](https://github.com/user-attachments/assets/24b5168e-71b3-4347-a502-7242beb8a7ab)
    ![image](https://github.com/user-attachments/assets/41fe5e14-9e14-4a7e-bfa7-3f5db0cfd7cb)
    ![image](https://github.com/user-attachments/assets/7b425933-0c3a-46b7-8715-ac5194ad666a)
    ![image](https://github.com/user-attachments/assets/6e5fab08-9f81-49a2-b78f-51579227fa64) .
    
  - Strong Scaling Speedup Plots
    ![image](https://github.com/user-attachments/assets/460e1573-1249-4685-b49a-a3d7633f3f90)
    ![image](https://github.com/user-attachments/assets/b8b755be-c40c-4d51-9962-3a1d4821f336)
    ![image](https://github.com/user-attachments/assets/8f90baf7-1f80-49f6-a469-c9d53842cdc8)
    ![image](https://github.com/user-attachments/assets/7a8f68f2-495f-426d-8803-319322c229f5)  .
    
  - Weak Scaling Plots (Combined on One Graph)
    ![image](https://github.com/user-attachments/assets/436e7872-c938-42b3-a73e-45faaa934647)  .
    

- **Comm:**
  
  - Strong Scaling Plots for Each Input Size
    ![image](https://github.com/user-attachments/assets/3afb6a97-329c-4a77-b066-88237cbcb18f)
    ![image](https://github.com/user-attachments/assets/357ca2c0-d2db-4f84-8c6a-409676b76d2f)
    ![image](https://github.com/user-attachments/assets/9904b45a-6c2a-4611-9f51-3656829fcdf7)
    ![image](https://github.com/user-attachments/assets/93c7d583-a44e-4282-9368-9908c6d68e4e)
    ![image](https://github.com/user-attachments/assets/d0fa6395-790c-4920-b6af-cbbb2ae3416a)
    ![image](https://github.com/user-attachments/assets/5878600e-4590-4f33-a1b2-f73da756ec1b)
    ![image](https://github.com/user-attachments/assets/d3455ba2-742e-4ff7-b931-6cf26ca243cf)  .
    
  - Strong Scaling Speedup Plots
    ![image](https://github.com/user-attachments/assets/ec4d1147-246c-4b57-a7af-6d9f52827183)
    ![image](https://github.com/user-attachments/assets/d8b1c7e0-6453-45ea-a245-71d903008f70)
    ![image](https://github.com/user-attachments/assets/bdbc6e6e-2420-4dc1-8bbb-52aecdaff6e8)
    ![image](https://github.com/user-attachments/assets/d2689944-3080-43fc-9024-68a6df8ccc91) .
      
  - Weak Scaling Plots (Combined on One Graph)
    ![image](https://github.com/user-attachments/assets/21d1a8b1-92e9-43ad-8c14-76d304b3e89c)  .


- **Comm_Large:**
  
  - Strong Scaling Plots for Each Input Size
    ![image](https://github.com/user-attachments/assets/30fa8e90-45c2-4659-8acd-3ac549fa480c)
    ![image](https://github.com/user-attachments/assets/88cd49c5-c954-4f45-b90f-7c03e410935b)
    ![image](https://github.com/user-attachments/assets/c4e443e5-47eb-45a1-8d11-efdf66ee37a5)
    ![image](https://github.com/user-attachments/assets/9e9896e4-14a7-47e7-9d64-9d4268fd598d)
    ![image](https://github.com/user-attachments/assets/2a772bbd-f1fc-40c5-ba87-fbe945d05b99)
    ![image](https://github.com/user-attachments/assets/dda8176e-5b6c-4249-93ab-151e4a55b4c3)
    ![image](https://github.com/user-attachments/assets/1566a511-d845-48d5-ab43-bcfdd4d680c0)  .
    
  - Strong Scaling Speedup Plots
    ![image](https://github.com/user-attachments/assets/d6b20e34-2504-4907-b98f-3edf88124c5d)
    ![image](https://github.com/user-attachments/assets/8937d18b-7a05-4ce5-9df0-25ba8eb2e5de)
    ![image](https://github.com/user-attachments/assets/f6cc7a1e-5209-4beb-a688-25b19c727c96)
    ![image](https://github.com/user-attachments/assets/b2af323b-ed65-4974-87dc-bc961e44b9bc)   .
    
  - Weak Scaling Plots (Combined on One Graph)
    ![image](https://github.com/user-attachments/assets/80def428-eb57-4a36-bafa-314dc92b20bb)  .


#### 2. Analysis of Results - Bitonic Sort (Ishaan)

1. Strong Scaling Analysis   

Large array sizes scale well: In the strong scaling plots, particularly for larger arrays like 67108864 and 268435456, the implementation demonstrates good scaling. Larger problem sizes allow for more efficient distribution of computation across processors, reducing the relative impact of communication overhead. With sufficient work per processor, the processors can focus more on sorting rather than communicating, resulting in improved speedup as more processors are added.

Small array sizes suffer from communication overhead: This is evident in the strong scaling plots, where communication costs dominate the runtime for small inputs, particularly with reverse and random input types. The relative cost of communication is too high in comparison to the computation time, which restricts scalability. For small arrays, the communication required between processors may overwhelm the benefits of parallel processing.

Input type matters: The algorithm performs best on sorted and nearly sorted inputs, which consistently show better strong scaling. For ordered data, fewer comparisons and exchanges are required, minimizing inter-processor communication. However, reverse and random inputs result in more frequent data exchanges, increasing communication overhead and limiting scalability.


2. Weak Scaling Analysis  

Ideally in the weak scaling plot, the average time per rank should remain relatively constant as more processors are added. In analyzing my plots however, I observe that the average time per rank increases with the number of processors, especially for the random input type. This tells me my algorithm is facing some overhead as more processors are added. I hypothesize that for random input types, there is higher communication and synchronization costs in the implementation. This input requires more data exchanges and sorting operations between processors, impacting the average time per processor to increase.  

3. Communication Overhead
    
Across both strong and weak scaling plots, communication overhead is the main limiting factor, especially for random input types. For the random input type, the MPI_Sendrecv calls increase substantially as processors exchange data and merge sorted subarrays, causing communication to dominate the runtime. This impact is especially pronounced in scenarios where data is highly disordered.

Sorted and nearly sorted inputs require fewer data exchanges, leading to better performance and scalability. The bitonic sort algorithm works more efficiently when the data is already partially ordered, as this reduces the communication needed to reach the final sorted state.

4. Speedup Analysis

An outlier was observed for the smallest array size, reaching an unrealistically high speedup of around 6000. This anomaly likely stems from measurement issues, where the timing overhead affects the results more than actual computation gains. For small datasets, measurement artifacts can inflate speedup values, so this data point can be considered an outlier and not a true representation of the algorithm’s performance.

For larger array sizes, the speedup grows at a slower rate. This tells me that as the data size increases, the communication overhead and the time for data transfer between processors start to outweigh the benefits of parallelization. For the largest array sizes, the speedup curve almost plateaus, indicating that adding more processors does not significantly improve performance. This also indicates that the overhead of managing inter-processor communication and data movement starts to dominate.


#### 1. Merge Sort (Sathvik)

**Example Graph:** 

- **Main:**
  
  - Strong Scaling Plots for Each Input Size:    
    ![Alt text](image.png)
    ![Alt text](image-1.png)
    ![Alt text](image-2.png)
    ![Alt text](image-3.png)
    ![Alt text](image-4.png)
    ![Alt text](image-5.png)
    ![Alt text](image-6.png)

  - Strong Scaling Speedup Plots
    ![Alt text](image-7.png)
    ![Alt text](image-8.png)
    ![Alt text](image-9.png)
    ![Alt text](image-10.png)

  - Weak Scaling Plots (Combined on One Graph)
    ![Alt text](image-11.png)
    

- **Comm:**
  
  - Strong Scaling Plots for Each Input Size
    ![Alt text](image-12.png)
    ![Alt text](image-13.png)
    ![Alt text](image-14.png)
    ![Alt text](image-15.png)
    ![Alt text](image-16.png)
    ![Alt text](image-17.png)
    ![Alt text](image-18.png)
    
  - Strong Scaling Speedup Plots
    ![Alt text](image-19.png)
    ![Alt text](image-20.png)
    ![Alt text](image-21.png)
    ![Alt text](image-22.png)
      
  - Weak Scaling Plots (Combined on One Graph)
    ![Alt text](image-23.png)
    


- **Comm_Large:**
  
  - Strong Scaling Plots for Each Input Size
    ![Alt text](image-24.png)
    ![Alt text](image-25.png)
    ![Alt text](image-26.png)
    ![Alt text](image-27.png)
    ![Alt text](image-28.png)
    ![Alt text](image-29.png)
    ![Alt text](image-30.png)
    
  - Strong Scaling Speedup Plots
    ![Alt text](image-31.png)
    ![Alt text](image-32.png)
    ![Alt text](image-33.png)
    ![Alt text](image-34.png)

  - Weak Scaling Plots (Combined on One Graph)
    ![Alt text](image-35.png)


#### 2. Analysis of Results

1. Strong Scaling Analysis 
The algorithm that I have written, Merge Sort, shows good and clear scalability for the runs with larger array sizes such as the ones with 2^26 and 2^28 elements where this increasing in processing decreases the overall execution time. The improvement in the time for execution stems from the algorithm having each processor handling the proper amount of data while also hosting an efficient merging process. On the other hand, with smaller sized arrays, the impact of the scaling isn't as noticeable as the lower amount of elements means that each processor's workload is too low to bring more computational gain than there is overhead from the communication. Essentially this means a lowered amount of performance improvenemnt for these arrays.

2. Weak Scaling Analysis
For this merge sort algorithm, whenever both the problem size and the input size increases proportionally, the algorithm performs very well on large sets of input. In these cases the execution times have stayed stable as the overall workload is evenly distributed creating a balanced performance. Based on the data, the merged data seems to be scaling somewhat logarithmically as it maintains efficient communication even with increased processors. Although, it is noticeable that there could be variances in the time based on the differences in local sorting times for highly varied data, but overall, weak scaling is still good.

3. Communication Overhead
In a parallel merge sort algorithm, the communication overhead exists within the distribution of the data amongst all of the processors as well as the merging of the subarrays. This overhead has a much more significant impact upon smaller inputs as the work that is split up amongst the processors isn't enought to outweigh this overhead by a lot. On the otherhand, with inputs that are much larger, the overhead becomes a welcome cost as the speedup caused by the distributed it work causes the sorting to occur much more quickly. In addition, in the algorithm, the use of Scatterv ensures that the distribution of the data as well as the communication during merging will be efficient.

One key item that I noticed is that the runs that I had done with the 128 processors seemed to have taken signifiantly more time than any of the other runs and I think this could be due to a variety of outside factors such as the workload of grace or just outliers in the data, but it doesn't seem to be representative of the algorithm.


#### 1. Sample Sort (Mustafa)

**Graphs:** 

Main (Strong Scaling):
![image](https://github.com/ishaan2947/Project_2024/blob/94801686447e97be27be335a62a7ec24620274fa/Sample_Sort/Graphs/Capture.JPG)

Comm (Strong Scaling):
![image](https://github.com/ishaan2947/Project_2024/blob/94801686447e97be27be335a62a7ec24620274fa/Sample_Sort/Graphs/Capture1.JPG)

Comp (Strong Scaling):
![image](https://github.com/ishaan2947/Project_2024/blob/94801686447e97be27be335a62a7ec24620274fa/Sample_Sort/Graphs/Capture2.JPG)


#### 2. Analysis of Results

Main:

Trend: All input types show a similar decreasing trend in the average time per rank as the number of processes increases.
Observations: The time per rank decreases sharply from 2 to 128 processes, indicating efficient load distribution and parallelization benefits. After about 128 processes, the time per rank flattens, showing diminishing returns with higher process counts.
Reasoning: The overhead of communication and coordination among more processes outweighs the benefits of further parallelization beyond a certain point, typical in parallel algorithms.

Comm:

Trend: For all inputs, the average time initially decreases as the number of processes increases, reaching a minimum around 128 to 256 processes. Beyond this point, time per rank begins to increase slightly.
Observations: This trend suggests that communication overhead in the MPI sample sort starts to dominate as the process count grows. As processes increase, the amount of inter-process communication required in sample sorting may introduce latency, especially in larger clusters.
Insight: The increase in time at higher process counts highlights the balance needed between computation and communication. Efficient sorting relies on optimal process count to minimize communication costs in MPI-based algorithms.

Comp:

Trend: The computation time decreases significantly with an increase in processes across all input types. This is consistent with the main trend, where parallelization reduces computation time effectively up to about 512 processes.
Observations: This graph shows that the computational workload is well distributed across processes. However, the rate of decrease slows down after a certain point, indicating again the diminishing returns of adding more processes.
Conclusion: The comp graph confirms that the algorithm scales well in terms of computation but suffers from increasing overhead at higher process counts.
Comparative Input Analysis
Random vs. Sorted vs. Perturbed vs. Reverse:
Across all graphs, the Sorted and Random inputs generally perform slightly better than Reverse and Perturbed. This pattern implies that the MPI sample sort algorithm is somewhat sensitive to the initial ordering of data, with less disorder in the input leading to faster performance.
For each input type, the differences are minimal at lower process counts but become slightly more apparent as process counts increase, especially in the "comm" graph where communication overhead plays a more significant role.

Overall:
The MPI sample sort algorithm shows good scalability with increasing processes, but performance gains diminish due to communication overhead, particularly at higher process counts (beyond 256 or 512). Sorted and Random inputs generally lead to slightly faster results, indicating that input order influences performance, although not drastically. The optimal number of processes seems to be around 128 to 256, balancing computation and communication efficiently.



#### 1. Radix Sort (Yusa)

**Graphs:** 

Main Random (Strong Scaling):
![image](https://github.com/user-attachments/assets/145ccd34-a6c5-4e0e-9086-702b419a557c)


Main Sorted (Strong Scaling):
![image](https://github.com/user-attachments/assets/1240d15f-eed9-48d7-bf5d-200a8c6dbde7)


Main Reverse (Strong Scaling):
![image](https://github.com/user-attachments/assets/ba9bcf73-fcf2-4f74-9828-cc06615a7d39)


Comp input size 6553 (Strong Scaling):
![image](https://github.com/user-attachments/assets/3fe928ae-f54e-4827-86ab-c35cb8aee686)


Comp input size 262144 (Strong Scaling):
![image](https://github.com/user-attachments/assets/c35e79d1-efb1-46e9-992a-a1d24a222b31)


Comp input size 1048576 (Strong Scaling):
![image](https://github.com/user-attachments/assets/1046303c-58ce-4dd2-a118-5f46815f03d4)


Comp input size 4194304 (Strong Scaling):
![image](https://github.com/user-attachments/assets/50b204f8-37e7-4715-a95b-05560e779be1)


Comp input size 16777216 (Strong Scaling):
![image](https://github.com/user-attachments/assets/115f283b-1aa5-4e20-a68e-dad1b96533ef)


Comp input size 67108864 (Strong Scaling):
![image](https://github.com/user-attachments/assets/412d848e-7d6a-4ad3-b5f3-82dd89613e9c)


Comp input size 268435456 (Strong Scaling):
![image](https://github.com/user-attachments/assets/fa78b5a1-1971-45f2-bd16-36f8b79fd239)


Speedup Random (Main)
![image](https://github.com/user-attachments/assets/a5494b33-9f82-4f46-aa63-6ba122d20b26)

Speedup Sorted (Main)
![image](https://github.com/user-attachments/assets/49b5451a-f2f0-48e3-9df8-ab17eb074a17)

Speedup Reverse (Main)
![image](https://github.com/user-attachments/assets/a2f5ef09-2b98-45e7-9f95-aba26613854a)

Speedup Perturbed 1% (Main)
![image](https://github.com/user-attachments/assets/282b5f61-1fb4-4c52-9d79-82535c5386eb)

Speedup Random (Communication)
![image](https://github.com/user-attachments/assets/05511d2e-dfc5-48d6-b341-5524bb7f307c)

Speedup Sorted (Communication)
![image](https://github.com/user-attachments/assets/53134068-e954-4047-b403-9e01e6f31f20)

Speedup Reverse (Communication)
![image](https://github.com/user-attachments/assets/57b6b005-9ffc-45a2-8853-32ed2ea750a9)

Speedup Perturbed 1% (Communication)
![image](https://github.com/user-attachments/assets/b721df64-bd78-4b3b-b69c-de6c3178ee94)

Speedup Random (Comp)
![image](https://github.com/user-attachments/assets/83c46198-a525-4927-ada2-133a8ca11310)

Speedup Sorted (Comp)
![image](https://github.com/user-attachments/assets/99818d69-d23e-48db-ba10-ce7867bd3c9e)

Speedup Reverse (Comp)
![image](https://github.com/user-attachments/assets/b03b8e49-6ce5-47e1-9cd2-d0ac144108d4)

Speedup Perturbed 1% (Comp)
![image](https://github.com/user-attachments/assets/e1762af7-dac1-4e45-8d94-e603b286bffa)




Main Random (Weak Scaling)
![image](https://github.com/user-attachments/assets/04e0df0a-372b-4143-8b2a-2b7373151a3e)

Main Sorted (Weak Scaling)
![image](https://github.com/user-attachments/assets/078fe029-de03-463a-af29-a3430fca2dcf)

Main Reverse (Weak Scaling)
![image](https://github.com/user-attachments/assets/197e8326-5268-427a-a75f-9901f580c54c)

Main Perturbed 1% (Weak Scaling)
![image](https://github.com/user-attachments/assets/e6b831aa-02aa-4f02-ad35-d7b2a403da37)


Communication Random (Weak Scaling)
![image](https://github.com/user-attachments/assets/59a83bb9-98c7-40bb-b728-278a6a8f4b03)

Communication Sorted (Weak Scaling)
![image](https://github.com/user-attachments/assets/5cbba818-aff8-448d-9555-d6d4e87009fc)

Communication Reverse (Weak Scaling)
![image](https://github.com/user-attachments/assets/c280b956-c884-4342-8b9e-e75a7dfcceea)

Communication Perturbed 1% (Weak Scaling)
![image](https://github.com/user-attachments/assets/ef30b879-0226-4986-828b-c93e6e353d9a)




#### 2. Analysis of Results Radix Sort


1. Strong Scaling Analysis:

Large input sizes scale well: The parallel Radix Sort implementation demonstrates effective strong scaling for larger input sizes, such as 2^24 2^24 and 2^28 2^28
  elements. As the number of processors increases, the average time per rank decreases significantly for these large datasets. This indicates that the workload is being efficiently divided among processors, allowing for concurrent processing of different portions of the data. The algorithm benefits from the parallelization of digit-wise sorting steps, which are computationally intensive for large arrays.

Diminishing returns at higher processor counts: While the time per rank decreases with more processors, the rate of improvement slows down beyond a certain point, typically around 256 to 512 processors. This plateau suggests that communication overhead and synchronization costs start to offset the benefits of adding more processors. The fixed overheads in the algorithm, such as broadcasting the maximum number of digits and gathering sorted subarrays, become more prominent as the per-processor workload decreases.

Small input sizes suffer from overhead: For smaller input sizes like 
2^16, 2^16 and 2^18 2^18, the strong scaling plots show minimal improvement or even slight increases in time per rank with more processors. This is due to the overhead of initializing MPI processes and the relative cost of communication compared to computation. With less data per processor, the time spent on communication and synchronization constitutes a larger fraction of the total execution time, negating the benefits of parallelism.

Input type impact is minimal: Unlike comparison-based sorting algorithms, Radix Sort's performance is less sensitive to the initial ordering of data. The strong scaling plots for different input types—Random, Sorted, Reverse Sorted, and 1% Perturbed—are closely aligned. This consistency occurs because Radix Sort processes each digit position uniformly, regardless of the data distribution. Therefore, the algorithm exhibits similar performance characteristics across different input types in strong scaling scenarios.

2. Weak Scaling Analysis:

Stable performance with increasing problem size: In the weak scaling analysis, where both the problem size and the number of processors increase proportionally, the average time per rank remains relatively constant for larger datasets. This indicates that the algorithm scales efficiently, maintaining consistent performance as the workload per processor stays the same. The Radix Sort algorithm effectively handles larger datasets without significant degradation in per-processor performance.

Slight increase in time for higher processor counts: Although the average time per rank is relatively stable, there is a slight upward trend as the number of processors increases beyond 256. This increase is attributed to the cumulative communication overhead associated with more processors. Each additional processor introduces more points of synchronization and communication, slightly increasing the total execution time.

Uniform performance across input types: Similar to the strong scaling results, the weak scaling plots show minimal variation across different input types. The algorithm's digit-wise processing ensures that the input data's initial order does not significantly impact the computation or communication patterns. As a result, the average time per rank remains consistent across Random, Sorted, Reverse Sorted, and 1% Perturbed inputs in weak scaling experiments.

3. Communication Overhead:

Communication costs become significant at high processor counts: Communication overhead is a critical factor affecting the parallel Radix Sort's performance, especially as the number of processors increases. The algorithm involves multiple collective operations, such as broadcasts and gathers, during each digit's sorting phase. These operations require synchronization and data exchange among all processors, which can introduce latency.

Impact on small input sizes: For smaller arrays, the communication overhead can dominate the total execution time. Since each processor handles a smaller portion of the data, the relative cost of communication (e.g., broadcasting the maximum number of digits or gathering sorted subarrays) increases. This overhead leads to less efficient scaling and can result in longer execution times despite increasing the number of processors.

Efficient communication patterns for large inputs: In contrast, for larger input sizes, the communication overhead is amortized over a more substantial computational workload per processor. The time spent on communication constitutes a smaller percentage of the total execution time, allowing the algorithm to scale more effectively. The use of optimized MPI collective operations helps mitigate communication costs, but their impact becomes more pronounced with higher processor counts.

4. Input Type Effects:

Minimal influence of input ordering: The Radix Sort algorithm's performance is largely independent of the initial data ordering. Since the algorithm processes each digit position systematically, the distribution or order of the input data does not significantly affect the number of operations required. This behavior is reflected in the performance graphs, where all input types exhibit similar execution times across various processor counts.

Consistency across different datasets: The slight variations observed among different input types are negligible compared to those in comparison-based sorting algorithms. This consistency makes Radix Sort a robust choice for parallel sorting tasks where the input data characteristics are unknown or highly variable.

5. Overall Observations and Speedup Analysis:

Effective parallelization for large-scale sorting: The parallel Radix Sort implementation demonstrates good scalability for large input sizes, making it suitable for high-performance computing environments where massive datasets are common. The algorithm efficiently leverages multiple processors to reduce computation time, particularly when the per-processor workload is substantial enough to outweigh communication overheads.

Diminishing returns due to communication overhead: As the number of processors increases beyond a certain threshold, the benefits of adding more processors diminish. The communication and synchronization costs begin to dominate, leading to a plateau in performance improvements. Identifying the optimal number of processors is crucial to maximize efficiency and minimize unnecessary overhead.
