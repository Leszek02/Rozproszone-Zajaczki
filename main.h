#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <set>
 
extern int BUNNY;
extern int BEAR;
//extern int* MEADOW;
extern int MEADOWSIZE;
 
extern std::vector<bool> MEADOWS
extern int rank;
extern int size;
extern pthread_t threadAnimal;
extern pthread_mutex_t stateMut;
extern MPI_Datatype MPI_PAKIET_T;
 
typedef struct {
    int id;
    int lamport;
} Message;
 
struct cmp {
    bool operator() (const Message &a, const Message &b) const {
            if(a.id == b.id) {
        return b.lamport > a.lamport;
        }
        return b.id > a.id;
    }
};
 
extern std::set<Message, cmp> partyQueue;
extern std::mutex mutexQueue;
 
#endif
 
//mpic++ main.cpp animal.cpp -o main
//mpiexec -n <number> ./main