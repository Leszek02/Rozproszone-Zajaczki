#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
 
extern int BUNNY;
extern int BEAR;
//extern int* MEADOW;
extern int MEADOWSIZE;
extern int MEADOWCOUNT;
 
extern int rank;
extern int size;
extern int hostCount;
extern int partyMeadow;
extern int alcohol;
extern int myHost;
extern bool partyStarted;
extern pthread_t threadAnimal;
extern pthread_mutex_t stateMut;
extern MPI_Datatype MPI_PAKIET_T;
 
typedef struct {
    int id;
    int lamport;
} Message;
 
extern std::vector<Message> inviteList;
extern std::vector<bool> meadows;
 
extern bool operator== (const Message &a, const Message &b); 
 
struct cmp {
    bool operator() (const Message a, const Message b) const {
        if(a.lamport == b.lamport) {
            return b.id > a.id;
        }
        return b.lamport > a.lamport;
    }
};
 
extern std::set<Message, cmp> partyQueue;
extern std::set<Message, cmp> hostGroup;
extern std::mutex mutexQueue;
extern std::mutex mutexLamport;
extern std::mutex mutexMeadow;
extern std::mutex mutexState;
 
#endif
 
//mpic++ main.cpp animal.cpp -o main
//mpiexec -n <number> ./main