#include "main.h"
#include "animal.h"
 
pthread_mutex_t recvMut = PTHREAD_MUTEX_INITIALIZER;
state_t state = REST;
std::mutex mutexQueue;
int perc = 0;
int lamport = 0;
int animals = BUNNY + BEAR;
int ackCount = 0;
int invited = 0;
 
std::set<Message, cmp> partyQueue;
 
void initRandom() {
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr)) * getpid();
    std::srand(seed);
}
 
void sendMessage(int& rank, message_t& type, int& toRank, int& lamport) {
    Message msg;
    msg.id = rank;
    msg.lamport = lamport;
    MPI_Send(&msg, 2, MPI_INT, toRank, type, MPI_COMM_WORLD);
    //std::cout << "Jestem zwierzak: " << rank << ", Wysylam cos do: " << toRank << std::endl;
}
 
bool compare(const Message& a, const Message& b) {
    if(a.id == b.id) {
        return b.lamport > a.lamport;
    }
    return b.id > a.id;
}
 
void insertQueue(const Message newAnimal) {
    std::unique_lock<std::mutex> lock(mutexQueue);
    partyQueue.insert(newAnimal);
}
 
void receiverThread() {
    Message msg;
    MPI_Status status;
    message_t message;
    while(1) {
        MPI_Recv(&msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch(status.MPI_TAG){
            case REQPARTY:
                //std::cout << rank << ": Dodaje zwierze nr " << msg.id << " do kolejki!" << std::endl;
                insertQueue(msg);
                message = ACKPARTY;
                sendMessage(rank, message, msg.id, lamport);
            case ACKPARTY:
                //std::cout << rank <<": Dostalem potwierdzenie od " << msg.id << std::endl;
                ackCount--;
        }
    }
}
 
void *mainLoop(void *ptr) {
    MPI_Status status;
 
    initRandom();
    std::thread(receiverThread).detach();
 
    while(1){
        switch(state) {
            case REST:
                perc = random()%100;
                lamport++;
                if (perc > 25) {
                    std::cout << rank << " i IMPREZA" << std::endl;
                    ackCount = animals - 1;
                    message_t message = REQPARTY;
                    for (int i = 0; i < animals; i++) {
                        if (i != rank) {
                            sendMessage(rank, message, i, lamport);
                        }
                    }
                    Message temp;
                    temp.id = rank;
                    temp.lamport = lamport;
                    insertQueue(temp);
                    state = WAITHOST;
                }
                else {
                    //std::cout << "Jestem " << (rank < BUNNY ? "zajacem" : "misiem") << " i wylosowalem liczbe: " << perc << std::endl;
                    sleep(rank + 1 + animals);
                    for (Message temp : partyQueue) {
                        std::cout << temp.id << "  ";
                    }
                    std::cout << std::endl;
                    while(1) { }
                    //TODO: Think what to do if someone is not partying (I guess sleep for a few seconds?)
                }
            case WAITHOST:
                while (ackCount > 0) {}
                std::cout << rank << ": dostalem potwierdzenie od wszystkich POG" << std::endl;
                sleep(rank + 1);
                for (Message temp : partyQueue) {
                        std::cout << temp.id << "  ";
                    }
                std::cout << std::endl;
                if (partyQueue.begin()->id == rank) {
                    state = WAITGROUP;
                }
                else {
                    while (!invited) {
                        state = WAITMEADOW;
                        break;
                    } //TODO: invited should be changed when TAKINGYOU is received 
                }
 
            case WAITGROUP: {
                std::vector<Message> inviteList;
                int inviteCount = 0;
                for (Message animal : partyQueue) {
                    if (inviteCount + (animal.id < BUNNY ? 1 : 4) <= MEADOWSIZE) {
                        inviteCount += (animal.id < BUNNY ? 1 : 4);
                        inviteList.push_back(animal);
                    }
                    if (inviteCount == MEADOWSIZE) {
                        for (Message invitation : partyQueue) {
                            if (std::find(inviteList.begin(), inviteList.end(), invitation) != inviteList.end()) {
                                message_t message = TAKINGYOU;
                                sendMessage(rank, message, invitation.id, lamport);
                            }
                            else {
                                message_t message = TAKINGTHEM;
                                sendMessage(rank, message, invitation.id, lamport);
                            }
                        }
                        state = WAITMEADOWHOST;
                        break;
                    }
                }
            }
 
            case WAITMEADOW:
                sleep(1);
 
            case WAITMEADOWHOST:
                sleep(1);
 
            case PARTY:
                sleep(1);
        }
    }
}