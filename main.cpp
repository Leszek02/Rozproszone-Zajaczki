#include "main.h"
#include "animal.h"
 
int BUNNY = 9;
int BEAR = 1;
 
int rank, size;
 
pthread_t threadAnimal;
pthread_mutex_t stateMut;
MPI_Datatype MPI_PAKIET_T;
 
void finalizuj() {
    pthread_mutex_destroy( &stateMut);
    pthread_join(threadAnimal, NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}
 
 
int main(int argc, char** argv) {
    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
 
 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
 
    pthread_create( &threadAnimal, NULL, mainLoop, 0);
 
    while(1){sleep(1);}
 
    finalizuj();
    return 0;
}