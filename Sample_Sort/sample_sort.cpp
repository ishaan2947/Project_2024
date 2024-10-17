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
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include <string>
#include <algorithm>
#include <vector>


void mpi_sample_sort(int *local_data, int local_n, int rank, int size) {
    
    CALI_MARK_BEGIN("local_sort");
    std::sort(local_data, local_data + local_n);
    CALI_MARK_END("local_sort");

    
    CALI_MARK_BEGIN("choose_splitters");


    int *splitters = nullptr;
    int *all_splitters = nullptr;
    if (rank == 0) {
        splitters = (int *)malloc((size - 1) * sizeof(int));
        all_splitters = (int *)malloc(size * (size - 1) * sizeof(int));
    }


    int step = local_n / size;
    std::vector<int> local_splitters;


    for (int i = 1; i < size; ++i) {
        local_splitters.push_back(local_data[i * step]);
    }


    MPI_Gather(local_splitters.data(), size - 1, MPI_INT, all_splitters, size - 1, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("choose_splitters");

    
    if (rank == 0) {

        std::vector<int> all_splitters_vec(all_splitters, all_splitters + size * (size - 1));
        std::sort(all_splitters_vec.begin(), all_splitters_vec.end());

        for (int i = 1; i < size; ++i) {
            splitters[i - 1] = all_splitters_vec[i * (size - 1)];

        }
        free(all_splitters);


    }
    MPI_Bcast(splitters, size - 1, MPI_INT, 0, MPI_COMM_WORLD);

    
    CALI_MARK_BEGIN("partition_data");


    std::vector<int> send_counts(size, 0);
    std::vector<int> send_displs(size, 0);
    std::vector<int> recv_counts(size, 0);
    std::vector<int> recv_displs(size, 0);


    std::vector<std::vector<int>> buckets(size);

    for (int i = 0; i < local_n; ++i) {
        int bucket_idx = 0;
        while (bucket_idx < size - 1 && local_data[i] > splitters[bucket_idx]) {
            ++bucket_idx;
        }
        buckets[bucket_idx].push_back(local_data[i]);
    }

    for (int i = 0; i < size; ++i) {
        send_counts[i] = buckets[i].size();
    }


    MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    send_displs[0] = 0;
    recv_displs[0] = 0;


    for (int i = 1; i < size; ++i) {
        send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
        recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
    }

    int total_send = send_displs[size - 1] + send_counts[size - 1];
    int total_recv = recv_displs[size - 1] + recv_counts[size - 1];


    std::vector<int> send_buffer(total_send);
    std::vector<int> recv_buffer(total_recv);

    int idx = 0;
    for (int i = 0; i < size; ++i) {
        for (int val : buckets[i]) {
            send_buffer[idx++] = val;
        }
    }
    MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displs.data(), MPI_INT,
                  recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_INT, MPI_COMM_WORLD);
    CALI_MARK_END("partition_data");

    
    CALI_MARK_BEGIN("merge_data");


    std::sort(recv_buffer.begin(), recv_buffer.end());
    for (int i = 0; i < total_recv; ++i) {
        local_data[i] = recv_buffer[i];
    }


    CALI_MARK_END("merge_data");


}

int main(int argc, char *argv[]) {
    int rank, size;
    int n = 1024; 
    int *data = NULL;
    int *local_data = NULL;
    int local_n;
    double start_time, end_time;

    
    MPI_Init(&argc, &argv);

    
    cali::ConfigManager mgr;
    mgr.add("runtime-report");
    mgr.start();

    
    CALI_MARK_BEGIN("main");

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    
    if (argc >= 2) {
        n = atoi(argv[1]);
    }

    std::string input_type = "random";
    if (argc >= 3) {
        input_type = argv[2];
    }

    
    if (n % size != 0) {
        if (rank == 0)
            printf("Array size must be divisible by number of processes.\n");
        MPI_Finalize();
        exit(0);
    }

    local_n = n / size;
    local_data = (int *)malloc(local_n * sizeof(int));

    
    if (rank == 0) {
        data = (int *)malloc(n * sizeof(int));

        
        CALI_MARK_BEGIN("data_init_runtime");

        
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

    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    MPI_Scatter(data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    
    MPI_Barrier(MPI_COMM_WORLD);


    start_time = MPI_Wtime();

    
    mpi_sample_sort(local_data, local_n, rank, size);

    
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();

    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    if (rank == 0) {
        MPI_Gather(MPI_IN_PLACE, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    
    free(local_data);

    
    mgr.flush();
    mgr.stop();

    
    adiak::init(NULL);
    adiak::launchdate();    // Launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", "Sample Sort"); // The name of the algorithm
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
