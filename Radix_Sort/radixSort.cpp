/******************************************************************************
 * FILE: radixSort.cpp
 * DESCRIPTION:
 *   MPI implementation of Radix Sort with Caliper instrumentation.
 * AUTHOR:
 *   Yusa Sagli
 ******************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include <string>

// Get the maximum value in the array for counting sort
int get_max(int *data, int n) {
    int max_val = data[0];
    for (int i = 1; i < n; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    return max_val;
}

// Counting sort for Radix Sort
void counting_sort(int *data, int n, int exp) {
    int *output = (int *)malloc(n * sizeof(int));
    int count[10] = {0};

    for (int i = 0; i < n; i++) {
        count[(data[i] / exp) % 10]++;
    }

    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        output[count[(data[i] / exp) % 10] - 1] = data[i];
        count[(data[i] / exp) % 10]--;
    }

    for (int i = 0; i < n; i++) {
        data[i] = output[i];
    }

    free(output);
}

// Local Radix Sort function
void radix_sort_local(int *data, int n) {
    int max_val = get_max(data, n);

    for (int exp = 1; max_val / exp > 0; exp *= 10) {
        counting_sort(data, n, exp);
    }
}

// MPI Radix Sort function
void mpi_radix_sort(int *local_data, int local_n, int rank, int size) {
    int global_max;

    // Compute the local maximum
    int local_max = get_max(local_data, local_n);

    // Compute the global maximum
    MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    // Perform radix sort on each digit
    for (int exp = 1; global_max / exp > 0; exp *= 10) {
        // Perform local counting sort for the current digit
        counting_sort(local_data, local_n, exp);

        // Gather all sorted subarrays at the root process
        int *gathered_data = NULL;
        if (rank == 0) {
            gathered_data = (int *)malloc(local_n * size * sizeof(int));
        }

        MPI_Gather(local_data, local_n, MPI_INT, gathered_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

        // Scatter the data back to all processes after sorting at root
        if (rank == 0) {
            counting_sort(gathered_data, local_n * size, exp);
        }

        MPI_Scatter(gathered_data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            free(gathered_data);
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int n = 1024; // Default input size
    int *data = NULL;
    int *local_data = NULL;
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

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get command line arguments
    if (argc >= 2) {
        n = atoi(argv[1]);
    }

    std::string input_type = "random";
    if (argc >= 3) {
        input_type = argv[2];
    }

    // Ensure that n is divisible by size
    if (n % size != 0) {
        if (rank == 0)
            printf("Array size must be divisible by number of processes.\n");
        MPI_Finalize();
        exit(0);
    }

    local_n = n / size;
    local_data = (int *)malloc(local_n * sizeof(int));

    // Data Initialization
    if (rank == 0) {
        data = (int *)malloc(n * sizeof(int));

        // Data initialization region
        CALI_MARK_BEGIN("data_init_runtime");

        // Initialize data based on input_type
        if (input_type == "random") {
            srand(time(NULL));
            for (int i = 0; i < n; i++) {
                data[i] = rand() % n;
            }
        } else if (input_type == "sorted") {
            for (int i = 0; i < n; i++) {
                data[i] = i;
            }
        } else if (input_type == "reverse") {
            for (int i = 0; i < n; i++) {
                data[i] = n - i;
            }
        } else {
            srand(time(NULL));
            for (int i = 0; i < n; i++) {
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

    // Perform Radix Sort
    mpi_radix_sort(local_data, local_n, rank, size);

    // Synchronize all processes after sorting
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    // Gather sorted data
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    if (rank == 0) {
        MPI_Gather(MPI_IN_PLACE, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    if (rank == 0) {
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
    adiak::value("algorithm", "Radix"); // The name of the algorithm
    adiak::value("programming_model", "MPI"); // Programming model used
    adiak::value("data_type", "int"); // Data type of input elements
    adiak::value("size_of_data_type", sizeof(int)); // Size of data type in bytes
    adiak::value("input_size", n); // Number of elements in input dataset
    adiak::value("input_type", input_type); // Type of input data
    adiak::value("num_procs", size); // Number of processors (MPI ranks)
    adiak::value("scalability", "strong"); // Scalability type ("strong" or "weak")
    adiak::value("group_num", 21); // Group number
    adiak::value("implementation_source", "Handwritten"); // Source of implementation

    CALI_MARK_END("main");

    MPI_Finalize();
    return 0;
}

