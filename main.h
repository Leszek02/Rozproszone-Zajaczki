#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>

extern int BUNNY;
extern int BEAR;


extern int rank;
extern int size;
extern int ackCount;
extern int lamport;
extern pthread_t threadAnimal;
extern pthread_mutex_t stateMut;
extern MPI_Datatype MPI_PAKIET_T;

#endif

//mpic++ main.cpp animal.cpp -o main
//mpiexec -n <number> ./main