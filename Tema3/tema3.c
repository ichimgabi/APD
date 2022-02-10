#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int num_neigh[1];
static int *neigh;

//Function taken from lab10
void read_neighbours(int rank) {
    FILE *fp;
    char file_name[30];
    sprintf(file_name, "cluster%d.txt", rank);

    fp = fopen(file_name, "r");
    fscanf(fp, "%d", &num_neigh[0]);

    neigh = malloc(sizeof(int) * num_neigh[0]);

    for (int i = 0; i < num_neigh[0]; i++)
        fscanf(fp, "%d", &neigh[i]);
}

int get_cluster(char *topology, int rank) {
    int cluster;
    for(int i = 0; i < 1000; i++) {
        if(topology[i] == '0')
            cluster = 0;
        if(topology[i] == '1')
            cluster = 1;
        if(topology[i] == '2')
            cluster = 2;
        if(topology[i] == (rank+'0')) 
            return cluster;
    }
    return -1;
}

int get_cluster_number_neigh(char *topology, int cluster, int numtasks) {
    int n_0 = 0, n_1 = 0;
    int i = 0;

    if(topology[i] == '0') {
        i++;
        while(topology[i] != '1') {
            if(topology[i] != ':' && topology[i] != ' ' && topology[i] != ',')
                n_0++;
            i++;
        }
    }
    if(topology[i] == '1') {
        i++;
        while(topology[i] != '2') {
            if(topology[i] != ':' && topology[i] != ' ' && topology[i] != ',')
                n_1++;
            i++;
        }
    }
    
    if(cluster == 0)
        return n_0;
    else if(cluster == 1)
        return n_1;
    else
        return numtasks - n_0 - n_1 - 3;
}

int main (int argc, char *argv[]) {
    int numtasks, rank;
    int *val_primita, array_size[1];
    char topology[1000];
    char charValue[10];

    int result_size[1];
    int *result;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0 || rank == 1 || rank == 2)
        read_neighbours(rank);

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0) {
        //Building the topology

        strcpy(topology, "0:");
        for(int i = 0; i < num_neigh[0]; i++) {
            sprintf(charValue, "%d", neigh[i]);
            strcat(topology, charValue);
            if(i != num_neigh[0] - 1)
                strcat(topology, ",");
            else
                strcat(topology, " ");
        }

        MPI_Recv(array_size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        val_primita = malloc(sizeof(int) * array_size[0]);
        MPI_Recv(val_primita, array_size[0], MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        strcat(topology, "1:");
        for(int i = 0; i < array_size[0]; i++) {
            sprintf(charValue, "%d", val_primita[i]);
            strcat(topology, charValue);
            if(i != array_size[0] - 1)
                strcat(topology, ",");
            else
                strcat(topology, " ");
        }

        MPI_Recv(array_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        val_primita = realloc(val_primita, sizeof(int) * array_size[0]);
        MPI_Recv(val_primita, array_size[0], MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        strcat(topology, "2:");
        for(int i = 0; i < array_size[0]; i++) {
            sprintf(charValue, "%d", val_primita[i]);
            strcat(topology, charValue);
            if(i != array_size[0] - 1)
                strcat(topology, ",");
        }

        printf("%d -> %s\n", rank, topology);

        MPI_Send(topology, 1000, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(topology, 1000, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Send(topology, 1000, MPI_CHAR, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }

        //Computing stage

        int vector_size[1];
        vector_size[0] = atoi(argv[1]);
        int *vector;
        vector = malloc(sizeof(int) * vector_size[0]);
        int *partial_vector;
        int partial_vector_size[1];
        partial_vector_size[0] = vector_size[0] / (numtasks - 3);
        partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
        int current_element = 0;

        //In case the vector doesn't split evenly among the workers
        int remaining_partial_vector_size[1];
        remaining_partial_vector_size[0] = vector_size[0] % (numtasks - 3);

        for(int i = 0; i < vector_size[0]; i++)
            vector[i] = i;

        //Sends data to the workers of process 0
        for(int i = 0; i < num_neigh[0]; i++) {
            for(int j = 0; j < partial_vector_size[0]; j++) {
                partial_vector[j] = vector[current_element];
                current_element++;
            }
            if(remaining_partial_vector_size[0] != 0) {
                partial_vector_size[0]++;
                partial_vector = realloc(partial_vector, sizeof(int) * partial_vector_size[0]);
                partial_vector[partial_vector_size[0] - 1] = vector[current_element];
                current_element++;
                remaining_partial_vector_size[0]--;
            }
            MPI_Send(partial_vector_size, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
            MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }
        free(partial_vector);

        //Sends data to process 1
        partial_vector_size[0] = vector_size[0] / (numtasks - 3) * get_cluster_number_neigh(topology, 1, numtasks);
        partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
        for(int i = 0; i < partial_vector_size[0]; i++) {
            partial_vector[i] = vector[current_element];
            current_element++;
        }
        if(remaining_partial_vector_size[0] != 0) {
            partial_vector_size[0]++;
            partial_vector = realloc(partial_vector, sizeof(int) * partial_vector_size[0]);
            partial_vector[partial_vector_size[0] - 1] = vector[current_element];
            current_element++;
            remaining_partial_vector_size[0]--;
        }
        MPI_Send(partial_vector_size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,1)\n", rank);
        free(partial_vector);

        //Sends data to process 2
        partial_vector_size[0] = vector_size[0] / (numtasks - 3) * get_cluster_number_neigh(topology, 2, numtasks);
        partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
        for(int i = 0; i < partial_vector_size[0]; i++) {
            partial_vector[i] = vector[current_element];
            current_element++;
        }
        if(remaining_partial_vector_size[0] != 0) {
            partial_vector_size[0]++;
            partial_vector = realloc(partial_vector, sizeof(int) * partial_vector_size[0]);
            partial_vector[partial_vector_size[0] - 1] = vector[current_element];
            current_element++;
            remaining_partial_vector_size[0]--;
        }
        MPI_Send(partial_vector_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);
        MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,2)\n", rank);

        free(partial_vector);
        free(vector);
        vector = malloc(sizeof(int) * vector_size[0]);
        current_element = 0;

        //Receives final data from workers
        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Recv(partial_vector_size, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
            MPI_Recv(partial_vector, partial_vector_size[0], MPI_INT, neigh[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int j = 0; j < partial_vector_size[0]; j++) {
                vector[current_element] = partial_vector[j];
                current_element++;
            }
            free(partial_vector);
        }

        //Receives final data from the other coordinators
        for(int i = 0; i < get_cluster_number_neigh(topology, 1, numtasks); i++) {
            MPI_Recv(partial_vector_size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
            MPI_Recv(partial_vector, partial_vector_size[0], MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int j = 0; j < partial_vector_size[0]; j++) {
                vector[current_element] = partial_vector[j];
                current_element++;
            }
            free(partial_vector);
        }

        for(int i = 0; i < get_cluster_number_neigh(topology, 2, numtasks); i++) {
            MPI_Recv(partial_vector_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
            MPI_Recv(partial_vector, partial_vector_size[0], MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int j = 0; j < partial_vector_size[0]; j++) {
                vector[current_element] = partial_vector[j];
                current_element++;
            }
            free(partial_vector);
        }

        result_size[0] = vector_size[0];
        result = malloc(sizeof(int) * result_size[0]);
        result = vector;
    }

    if(rank == 1) {
        //Building the topology
        MPI_Send(num_neigh, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);
        MPI_Send(neigh, num_neigh[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);

        MPI_Recv(topology, 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("%d -> %s\n", rank, topology);

        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Send(topology, 1000, MPI_CHAR, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }

        //Computing stage

        //Receives data from process 0
        int *vector, vector_size[1];
        int current_element = 0;
        MPI_Recv(vector_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        vector = malloc(sizeof(int) * vector_size[0]);
        MPI_Recv(vector, vector_size[0], MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Send data to wrokers
        int *partial_vector;
        int partial_vector_size[1];
        partial_vector_size[0] = vector_size[0] / num_neigh[0];
        partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
        for(int i = 0; i < num_neigh[0]; i++) {
            for(int j = 0; j < partial_vector_size[0]; j++) {
                partial_vector[j] = vector[current_element];
                current_element++;
            }
            MPI_Send(partial_vector_size, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
            MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }

        //Receives final data from workers & sends it to process 0
        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Recv(partial_vector, partial_vector_size[0], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(partial_vector_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,0)\n", rank);
            MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,0)\n", rank);
        }
    }

    if(rank == 2) {
        //Building the topology
        MPI_Send(num_neigh, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);
        MPI_Send(neigh, num_neigh[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("M(%d,0)\n", rank);

        MPI_Recv(topology, 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("%d -> %s\n", rank, topology);

        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Send(topology, 1000, MPI_CHAR, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }

        //Computing stage

        //Receives data from process 0
        int *vector, vector_size[1];
        int current_element = 0;
        MPI_Recv(vector_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        vector = malloc(sizeof(int) * vector_size[0]);
        MPI_Recv(vector, vector_size[0], MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //Sends data to workers
        int *partial_vector;
        int partial_vector_size[1];
        partial_vector_size[0] = vector_size[0] / num_neigh[0];
        partial_vector = malloc(sizeof(int) * partial_vector_size[0]);
        for(int i = 0; i < num_neigh[0]; i++) {
            for(int j = 0; j < partial_vector_size[0]; j++) {
                partial_vector[j] = vector[current_element];
                current_element++;
            }
            MPI_Send(partial_vector_size, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
            MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, neigh[i]);
        }

        //Receives final data from workers & sends it to process 0
        for(int i = 0; i < num_neigh[0]; i++) {
            MPI_Recv(partial_vector, partial_vector_size[0], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(partial_vector_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,0)\n", rank);
            MPI_Send(partial_vector, partial_vector_size[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,0)\n", rank);
        }

    }

    for(int i = 3; i < numtasks; i++) {
        if(rank == i){
            MPI_Recv(topology, 1000, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%d -> %s\n", rank, topology);
            int cluster = get_cluster(topology, rank);

            //Computing stage

            //Each worker computes the final vector for the data they received
            int *vector, vector_size[1];
            MPI_Recv(vector_size, 1, MPI_INT, cluster, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector = malloc(sizeof(int) * vector_size[0]);
            MPI_Recv(vector, vector_size[0], MPI_INT, cluster, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int i = 0; i < vector_size[0]; i++)
                vector[i] *= 2;

            //Sending the result to the coordonators
            if(cluster == 0) {
                MPI_Send(vector_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                printf("M(%d,0)\n", rank);
            }
            MPI_Send(vector, vector_size[0], MPI_INT, cluster, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, cluster);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0) {
        printf("Rezultat: ");
        for(int i = 0; i < result_size[0]; i++)
            printf("%d ", result[i]);
    }

    MPI_Finalize();
    return 0;
}
