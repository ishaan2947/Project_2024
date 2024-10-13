/******************************************************************************
 * FILE: mpi_mm.cpp
 * DESCRIPTION:
 *   MPI implementation of Sample Sort with Caliper instrumentation.
 * AUTHOR:
 *   Mustafa Tekin
 * LAST REVISED:
 *   ...
 ******************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>

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

    // Start timing after local sort
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    free(local_data);

    // Flush and stop Caliper
    mgr.flush();
    mgr.stop();

    MPI_Finalize();
    return 0;
}