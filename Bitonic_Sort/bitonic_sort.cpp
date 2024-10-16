/******************************************************************************
 * FILE: mpi_mm.cpp
 * DESCRIPTION:
 *   MPI implementation of Bitonic Sort with Caliper instrumentation.
 * AUTHOR:
 *   Ishaan Nigam
 ******************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>                   // For log2()
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include <string>

// Comparison function for compare-exchange
void compare_exchange(int *data1, int *data2, int count, int dir)
{
    for (int i = 0; i < count; i++)
    {
        if ((dir == 1 && data1[i] > data2[i]) || (dir == 0 && data1[i] < data2[i]))
        {
            int temp = data1[i];
            data1[i] = data2[i];
            data2[i] = temp;
        }
    }
}

// Local bitonic sort function
void bitonic_sort_local(int *data, int start, int length, int dir)
{
    if (length > 1)
    {
        int k = length / 2;
        bitonic_sort_local(data, start, k, 1);
        bitonic_sort_local(data, start + k, k, 0);

        // Perform bitonic merge
        int step = k;
        while (step > 0)
        {
            for (int i = start; i < start + length - step; i++)
            {
                if ((i - start) % (2 * step) < step)
                {
                    if ((dir == 1 && data[i] > data[i + step]) ||
                        (dir == 0 && data[i] < data[i + step]))
                    {
                        int temp = data[i];
                        data[i] = data[i + step];
                        data[i + step] = temp;
                    }
                }
            }
            step /= 2;
        }
    }
}

// MPI Bitonic sort function
void mpi_bitonic_sort(int *local_data, int local_n, int rank, int size)
{
    int *recv_data = (int *)malloc(local_n * sizeof(int));
    int partner;

    CALI_MARK_BEGIN("mpi_bitonic_sort");

    int logp = (int)log2(size);

    for (int k = 0; k < logp; k++) // Stages
    {
        for (int j = k; j >= 0; j--) // Steps within stages
        {
            int mask = 1 << j;
            partner = rank ^ mask;

            // Determine sorting direction
            int dir = ((rank >> (k + 1)) & 1) == 0 ? 1 : 0; // 1 for ascending, 0 for descending

            CALI_MARK_BEGIN("comm");
            CALI_MARK_BEGIN("comm_large");
            MPI_Sendrecv(local_data, local_n, MPI_INT, partner, 0,
                        recv_data, local_n, MPI_INT, partner, 0,
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            CALI_MARK_END("comm_large");
            CALI_MARK_END("comm");

            CALI_MARK_BEGIN("comp_small");
            if (rank < partner)
            {
                if (dir == 1)
                {
                    compare_exchange(local_data, recv_data, local_n, dir);
                }
                else
                {
                    // Keep higher elements
                    compare_exchange(recv_data, local_data, local_n, dir);
                }
            }
            else
            {
                if (dir == 1)
                {
                    compare_exchange(recv_data, local_data, local_n, dir);
                }
                else
                {
                    // Keep higher elements
                    compare_exchange(local_data, recv_data, local_n, dir);
                }
            }
            CALI_MARK_END("comp_small");
        }
    }

    CALI_MARK_END("mpi_bitonic_sort");

    free(recv_data);
}

int main(int argc, char *argv[])
{
    int numtasks, rank;
    int n = 1024; // Default total number of elements
    int *data = NULL;
    int *local_data;
    int local_n;
    double start_time, end_time;

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Initialize Caliper ConfigManager
    cali::ConfigManager mgr;
    mgr.add("runtime-report");
    mgr.start();

    // Start main region
    CALI_MARK_BEGIN("main");

    // Get command line arguments
    if (argc >= 2)
    {
        n = atoi(argv[1]);
    }

    std::string input_type = "random";
    if (argc >= 3)
    {
        input_type = argv[2];
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Ensure that numtasks is a power of two
    if ((numtasks & (numtasks - 1)) != 0)
    {
        if (rank == 0)
            printf("Number of processes must be a power of two.\n");
        MPI_Finalize();
        exit(0);
    }

    // Ensure that n is divisible by numtasks
    if (n % numtasks != 0)
    {
        if (rank == 0)
            printf("Array size must be divisible by number of processes.\n");
        MPI_Finalize();
        exit(0);
    }

    local_n = n / numtasks;
    local_data = (int *)malloc(local_n * sizeof(int));

    // Data Initialization
    if (rank == 0)
    {
        data = (int *)malloc(n * sizeof(int));

        // Data initialization region
        CALI_MARK_BEGIN("data_init_runtime");

        // Initialize data based on input_type
        if (input_type == "random")
        {
            // Random data
            srand(time(NULL));
            for (int i = 0; i < n; i++)
            {
                data[i] = rand() % n;
            }
        }
        else if (input_type == "sorted")
        {
            // Sorted data
            for (int i = 0; i < n; i++)
            {
                data[i] = i;
            }
        }
        else if (input_type == "reverse")
        {
            // Reverse sorted data
            for (int i = 0; i < n; i++)
            {
                data[i] = n - i;
            }
        }
        else if (input_type == "nearly_sorted")
        {
            // Nearly sorted data
            for (int i = 0; i < n; i++)
            {
                data[i] = i;
            }
            int num_swaps = n / 100; // 1% perturbation
            for (int i = 0; i < num_swaps; i++)
            {
                int idx1 = rand() % n;
                int idx2 = rand() % n;
                int temp = data[idx1];
                data[idx1] = data[idx2];
                data[idx2] = temp;
            }
        }
        else
        {
            // Default to random
            srand(time(NULL));
            for (int i = 0; i < n; i++)
            {
                data[i] = rand() % n;
            }
        }

        CALI_MARK_END("data_init_runtime");
    }

    // Distribute data to all processes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Synchronize all processes before starting the timer
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // Local bitonic sort
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    bitonic_sort_local(local_data, 0, local_n, 1);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Perform the MPI bitonic sort
    mpi_bitonic_sort(local_data, local_n, rank, numtasks);

    // Synchronize all processes after sorting
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    // Gather sorted data
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    if (rank == 0)
    {
        MPI_Gather(MPI_IN_PLACE, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    if (rank == 0)
    {
        printf("Time taken: %f seconds\n", end_time - start_time);
        free(data);
    }

    free(local_data);

    // Flush and stop Caliper
    mgr.flush();
    mgr.stop();

    // Adiak metadata collection
    adiak::init(NULL);
    adiak::launchdate();    // Launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "Bitonic"); // The name of the algorithm
    adiak::value("programming_model", "MPI"); // Programming model used
    adiak::value("data_type", "int"); // Data type of input elements
    adiak::value("size_of_data_type", sizeof(int)); // Size of data type in bytes
    adiak::value("input_size", n); // Number of elements in input dataset
    adiak::value("input_type", input_type); // Type of input data
    adiak::value("num_procs", numtasks); // Number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // Scalability type ("strong" or "weak")
    adiak::value("group_num", 21); // Group number
    adiak::value("implementation_source", "Handwritten"); // Source of implementation

    CALI_MARK_END("main");

    MPI_Finalize();
    return 0;
}
