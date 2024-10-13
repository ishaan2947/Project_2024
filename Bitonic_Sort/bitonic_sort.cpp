/******************************************************************************
 * FILE: mpi_mm.cpp
 * DESCRIPTION:
 *   MPI implementation of Bitonic Sort with Caliper instrumentation.
 * AUTHOR:
 *   Ishaan Nigam
 * LAST REVISED:
 *   ...
 ******************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>                   // For log2()
#include <caliper/cali.h>
#include <caliper/cali-manager.h>

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

            CALI_MARK_BEGIN("communication");
            MPI_Sendrecv(local_data, local_n, MPI_INT, partner, 0,
                         recv_data, local_n, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            CALI_MARK_END("communication");

            CALI_MARK_BEGIN("compare_exchange");
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
            CALI_MARK_END("compare_exchange");
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

    // Get command line arguments
    if (argc >= 2)
    {
        n = atoi(argv[1]);
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
    // needed for algorithm to proerply work 
    if (n % numtasks != 0)
    {
        if (rank == 0)
            printf("Array size must be divisible by number of processes.\n");
        MPI_Finalize();
        exit(0);
    }

    local_n = n / numtasks;
    local_data = (int *)malloc(local_n * sizeof(int));

    if (rank == 0)
    {
        data = (int *)malloc(n * sizeof(int));

        // Initialize data (random data by default)
        srand(time(NULL));
        for (int i = 0; i < n; i++)
        {
            data[i] = rand() % n;
        }

        // Uncomment one of the following code blocks to initialize different input types

        // Sorted data
        /*
        for (int i = 0; i < n; i++)
        {
            data[i] = i;
        }
        */

        // Reverse sorted data
        /*
        for (int i = 0; i < n; i++)
        {
            data[i] = n - i;
        }
        */

        // Nearly sorted data (1% perturbation)
        /*
        for (int i = 0; i < n; i++)
        {
            data[i] = i;
        }
        int num_swaps = n / 100;
        for (int i = 0; i < num_swaps; i++)
        {
            int idx1 = rand() % n;
            int idx2 = rand() % n;
            int temp = data[idx1];
            data[idx1] = data[idx2];
            data[idx2] = temp;
        }
        */
    }

    // Distribute data to all processes
    MPI_Scatter(data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Local bitonic sort
    CALI_MARK_BEGIN("local_sort");
    bitonic_sort_local(local_data, 0, local_n, 1);
    CALI_MARK_END("local_sort");

    // Start timing after local sort
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // Perform the MPI bitonic sort
    mpi_bitonic_sort(local_data, local_n, rank, numtasks);

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    // Gather sorted data
    if (rank == 0)
    {
        MPI_Gather(MPI_IN_PLACE, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

        printf("Time taken: %f seconds\n", end_time - start_time);

        free(data);
    }
    else
    {
        MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }

    free(local_data);

    // Flush and stop Caliper
    mgr.flush();
    mgr.stop();

    MPI_Finalize();
    return 0;
}
