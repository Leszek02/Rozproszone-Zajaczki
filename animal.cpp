#include "main.h"
#include "animal.h"

pthread_mutex_t recvMut = PTHREAD_MUTEX_INITIALIZER;
state_t state = REST;

void initRandom() {
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr)) * getpid();
    std::srand(seed);
}

void *mainLoop(void *ptr)
{
    int perc;
    MPI_Status status;

    initRandom();

    while(1){
        switch(state) {
            case REST:
                perc = random()%100;
                std::cout << "Jestem " << (rank < BUNNY ? "zajacem" : "misiem") << " i wylosowalem liczbe: " << perc << std::endl;
                sleep(4);
        }
    }
}