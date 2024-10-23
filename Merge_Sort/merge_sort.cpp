#include <iostream>
#include <vector>
#include <algorithm>
#include <mpi.h>
#include <adiak.hpp>
#include <caliper/cali.h>
#include <string>
#include <cstdlib>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    // Begin Caliper main region
    CALI_MARK_BEGIN("main");

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Initialize Caliper and Adiak
    cali_init();
    adiak::init(NULL);

    // Get the rank and size of the MPI world
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Define variables for Adiak metadata
    string algorithm = "merge";
    string programmingModel = "mpi";
    string dataType = "int";
    int dataTypeSize = sizeof(int);
    long long inputSize = 0;
    string inputType = "Random"; // Default input type
    int numProcs = size;
    string scalability = "strong"; // Adjust if needed
    int groupNumber = 21; // Your group number
    string implementationSource = "handwritten"; // As per project requirements

    // Parse command-line arguments
    if (argc >= 2) {
        inputSize = atoll(argv[1]);
        if (argc >= 3) {
            inputType = argv[2];
        }
    } else {
        if (rank == 0) {
            cerr << "Usage: " << argv[0] << " input_size [input_type]" << endl;
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Determine input type and broadcast to all processes
    if (rank == 0) {
        // Validate inputType
        if (inputType != "Random" && inputType != "Sorted" && inputType != "ReverseSorted" && inputType != "1_perc_perturbed") {
            cerr << "Unknown input_type: " << inputType << endl;
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }
    char inputTypeCStr[50];
    if (rank == 0) {
        strncpy(inputTypeCStr, inputType.c_str(), 50);
    }
    // Broadcast inputType (small communication, not annotated)
    MPI_Bcast(inputTypeCStr, 50, MPI_CHAR, 0, MPI_COMM_WORLD);
    inputType = string(inputTypeCStr);

    // Collect Adiak metadata
    adiak::launchdate();
    adiak::libraries();

    // Construct the command line used
    string cmdLine = argv[0];
    for (int i = 1; i < argc; ++i) {
        cmdLine += " ";
        cmdLine += argv[i];
    }
    adiak::value("cmdline", cmdLine);
    adiak::clustername();
    adiak::value("algorithm", algorithm);
    adiak::value("programming_model", programmingModel);
    adiak::value("data_type", dataType);
    adiak::value("size_of_data_type", dataTypeSize);
    adiak::value("input_size", inputSize);
    adiak::value("input_type", inputType);
    adiak::value("num_procs", numProcs);
    adiak::value("scalability", scalability);
    adiak::value("group_num", groupNumber);
    adiak::value("implementation_source", implementationSource);

    // Initialize local data variables
    vector<int> localData;
    int localSize = 0;

    // Data initialization on root process
    CALI_MARK_BEGIN("data_init_runtime");
    vector<int> data;
    vector<int> sendCounts(size);
    vector<int> displacements(size);

    if (rank == 0) {
        data.resize(inputSize);
        // Generate data based on inputType
        if (inputType == "Random") {
            srand(42);
            for (long long i = 0; i < inputSize; ++i) {
                data[i] = rand();
            }
        } else if (inputType == "Sorted") {
            for (long long i = 0; i < inputSize; ++i) {
                data[i] = i;
            }
        } else if (inputType == "ReverseSorted") {
            for (long long i = 0; i < inputSize; ++i) {
                data[i] = inputSize - i;
            }
        } else if (inputType == "1_perc_perturbed") {
            for (long long i = 0; i < inputSize; ++i) {
                data[i] = i;
            }
            int numSwaps = inputSize / 100;
            srand(42);
            for (int i = 0; i < numSwaps; ++i) {
                long long idx1 = rand() % inputSize;
                long long idx2 = rand() % inputSize;
                swap(data[idx1], data[idx2]);
            }
        } else {
            cerr << "Unknown input_type: " << inputType << endl;
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        // Calculate sendCounts and displacements for scattering
        long long avgSize = inputSize / size;
        long long remainder = inputSize % size;
        long long offset = 0;
        for (int i = 0; i < size; ++i) {
            sendCounts[i] = avgSize + (i < remainder ? 1 : 0);
            displacements[i] = offset;
            offset += sendCounts[i];
        }

        // Print a sample of the initial data
        cout << "Sample of initial data:" << endl;
        for (int i = 0; i < min(10LL, inputSize); ++i) {
            cout << data[i] << " ";
        }
        cout << endl;
    }
    CALI_MARK_END("data_init_runtime");

    // Broadcast sendCounts to all processes (small communication, not annotated)
    MPI_Bcast(sendCounts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
    localSize = sendCounts[rank];
    localData.resize(localSize);

    // Distribute data among processes
    CALI_MARK_BEGIN("comm");
    MPI_Scatterv(rank == 0 ? data.data() : NULL, sendCounts.data(), displacements.data(), MPI_INT,
                 localData.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm");

    // Perform local sorting
    CALI_MARK_BEGIN("comp_large");
    sort(localData.begin(), localData.end());
    CALI_MARK_END("comp_large");

    // Merging phase
    int active = 1;
    int step = 1;
    while (step < size) {
        if (active) {
            if (rank % (2 * step) == 0) {
                if (rank + step < size) {
                    // Exchange sizes with neighbor (small communication, not annotated)
                    int recvSize;
                    MPI_Sendrecv(&localSize, 1, MPI_INT, rank + step, 0,
                                 &recvSize, 1, MPI_INT, rank + step, 0,
                                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    // Receive data from neighbor
                    vector<int> recvData(recvSize);
                    CALI_MARK_BEGIN("comm");
                    MPI_Recv(recvData.data(), recvSize, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    CALI_MARK_END("comm");

                    // Merge data
                    CALI_MARK_BEGIN("comp_large");
                    vector<int> mergedData(localSize + recvSize);
                    merge(localData.begin(), localData.end(), recvData.begin(), recvData.end(), mergedData.begin());
                    localData = mergedData;
                    localSize = localData.size();
                    CALI_MARK_END("comp_large");
                }
            } else if (rank % (2 * step) == step) {
                // Send size to neighbor (small communication, not annotated)
                MPI_Sendrecv(&localSize, 1, MPI_INT, rank - step, 0,
                             NULL, 0, MPI_INT, MPI_PROC_NULL, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Send data to neighbor
                CALI_MARK_BEGIN("comm");
                MPI_Send(localData.data(), localSize, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
                CALI_MARK_END("comm");
                active = 0; // Process becomes inactive
            }
        }
        MPI_Barrier(MPI_COMM_WORLD); // Synchronization (not annotated)
        step *= 2;
    }

    // Correctness check
    if (rank == 0) {
        bool isCorrect = true;
        for (size_t i = 1; i < localData.size(); ++i) {
            if (localData[i - 1] > localData[i]) {
                isCorrect = false;
                break;
            }
        }

        CALI_MARK_BEGIN("correctness_check");
        if (isCorrect) {
            cout << "Data is correctly sorted." << endl;
        } else {
            cout << "Data is not correctly sorted." << endl;
        }

        // Print a sample of the sorted data
        cout << "Sample of sorted data:" << endl;
        for (size_t i = 0; i < min(size_t(10), localData.size()); ++i) {
            cout << localData[i] << " ";
        }
        cout << endl;
        CALI_MARK_END("correctness_check");
    }

    // Finalize Adiak and Caliper
    adiak::fini();

    // Finalize MPI environment
    MPI_Finalize();

    // End Caliper main region
    CALI_MARK_END("main");

    return EXIT_SUCCESS;
}
