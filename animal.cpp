#include "main.h"
#include "animal.h"
 
pthread_mutex_t recvMut = PTHREAD_MUTEX_INITIALIZER;
state_t state = REST;
std::mutex mutexQueue;
std::mutex mutexLamport;
std::mutex mutexInvited;
std::mutex mutexState;
std::vector<Message> inviteList;
int perc = 0;
int lamport = 0;
int animals = BUNNY + BEAR;
int ackCount = 0;
int hostCount = 0;
int ackMead = 0;
int alcohol = 0;
int myHost = -1;
int partyMeadow = -1;
bool invited = false;
bool partyStarted = false;
bool partyOver = false;

std::set<Message, cmp> partyQueue;
std::set<Message, cmp> hostGroup;

//TODO: zmienianie invited musi by obarczone mutexem
 
void initRandom() {
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr)) * getpid();
    std::srand(seed);
}

void incrementLamport() {
    std::unique_lock<std::mutex> lock(mutexLamport);
    lamport++;
}

void synchroniseLamport(int receivedLamport) {
    std::unique_lock<std::mutex> lock(mutexLamport);
    lamport = std::max(lamport, receivedLamport) + 1;
}
 
bool operator== (const Message &a, const Message &b) {
        return b.id == a.id && b.lamport == a.lamport;
    }
 
void sendMessage(int& rank, message_t& type, const int& toRank, int& lamport) {
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

void changeState(state_t newState) {
    std::unique_lock<std::mutex> lock(mutexQueue);
    state = newState;
}
 
void insertQueue(const Message newAnimal) {
    std::unique_lock<std::mutex> lock(mutexQueue);
    partyQueue.insert(newAnimal);
}

void deleteQueue(const Message& deleteAnimal) {
    //TODO: Maybe check before if it is added to queue before? (maybe not necessary)
    //std::cout << "Chce usunac: " << deleteAnimal.id << std::endl;
    std::unique_lock<std::mutex> lock(mutexQueue);
    partyQueue.erase(deleteAnimal);
}

void printPartyQueue() {
    std::unique_lock<std::mutex> lock(mutexQueue);
    std::cout << rank << ": wyswietlam moja kolejke imprezowiczow: ";
    for (Message animal : partyQueue) {
        std::cout << "[" << animal.id << "; " << animal.lamport << "]";
    }
    std::cout << std::endl;
}

void printHostGroup() {
    printf("[%d;%d]: wyswietlam moja kolejke hostow: ", rank, lamport);
    for (Message animal : hostGroup) {
        std::cout << "[" << animal.id << "; " << animal.lamport << "]";
    }
    std::cout << std::endl;
}

void deleteHostGroup(int id) {
    for (auto host : hostGroup) {
        if (host.id == id) {
            hostGroup.erase(host);
            printf("[%d;%d] Usuwam hosta %d z kolejki hostow\n", rank, lamport, host.id);
            break;
        }
    }
}
 
int takeMeadow(){
    std::unique_lock<std::mutex> lock(mutexMeadow);
    for (int i = 0; i < MEADOWCOUNT; i++){
        if (meadows[i] == false){
            meadows[i] == true;
            return i;
        };
    }
    return -1;
}
 
void releaseMeadow(int i){
    std::unique_lock<std::mutex> lock(mutexMeadow);
    meadows[i] == false;
}
 
int getSpaceOccupied(int i){
    if(i < BUNNY){
        return 1;
    }else {
        return 4;
    }
}
 
void changeInvited() {
    std::unique_lock<std::mutex> lock(mutexInvited);
    if (invited) {
        invited = false;
    }
    else {
        invited = true;
    }

}

void updateQueue(){
    std::unique_lock<std::mutex> lock(mutexQueue);
    int spaceSum = 0;
    int i = 0;
    bool atLeastOneRabbit = false;
    std::vector<Message> goingToParty;
    for(const auto& iterator : partyQueue){
        if (spaceSum >= MEADOWSIZE){
            break;
        }
        int extraSpace = getSpaceOccupied(iterator.id);
        if (spaceSum + extraSpace <= MEADOWSIZE) {
            if (extraSpace == 1) {
                atLeastOneRabbit = true;
            }
            goingToParty.push_back(iterator);
            spaceSum += extraSpace;
        }
        else if (extraSpace == 1  && !atLeastOneRabbit && spaceSum < 4) {
            atLeastOneRabbit = true;
            goingToParty.push_back(iterator);
            spaceSum += extraSpace;
        }
            
    }
    if(!atLeastOneRabbit){
        //std::cout << "Nie zrobimy imprezy bez zadnego zajaca" << std::endl;
    } else {
        //std::cout << "Udalo sie zebrac chetnych wiec idziemy sobie" << std::endl;
        for (Message animal : goingToParty) {
            std::cout << rank << " Usuwam z kolejki: " << animal.id << std::endl;
            partyQueue.erase(animal);
        }
    }
}
 
void receiverThread() {
    Message msg;
    MPI_Status status;
    message_t message;
    while(1) {
        MPI_Recv(&msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        synchroniseLamport(msg.lamport);
        switch(status.MPI_TAG){
            case REQPARTY:
                //std::cout << rank << ": Dodaje zwierze nr " << msg.id << " do kolejki!" << std::endl;
                insertQueue(msg);
                message = ACKPARTY;
                sendMessage(rank, message, msg.id, lamport);
                break;
            case ACKPARTY:
                //std::cout << rank <<": Dostalem potwierdzenie od " << msg.id << std::endl;
                ackCount--;
                break;
            case TAKINGYOU: {
                //std::cout << "Obsluga TAKINGYOU dla id: " << rank << std::endl;
                //std::cout << rank << "  " << state << " Zostalem zaproszony na impreze cool\n";
                invited = true;
                Message temp;
                myHost = msg.id;
                temp.id = msg.id;
                temp.lamport = msg.lamport;
                hostGroup.insert(temp);
                //printf("[%d; %d]Dodaje hosta polany: %d\n",rank, lamport, msg.id);
                //printPartyQueue();
                break;
            }
            case TAKINGTHEM: {
                //std::cout << "Obsluga TAKINGTHEM dla id: " << rank << std::endl;
                invited = true;
                Message temp;
                temp.id = msg.id;
                temp.lamport = msg.lamport;
                hostGroup.insert(temp);
                //printf("[%d; %d]Dodaje hosta polany: %d\n",rank, lamport, msg.id);
                //printPartyQueue();
                break;
            }
            case REQMEADOW:
                //std::cout << "dajcie polane " << std::endl;
                break;
            case ACKMEADOW:
                //std::cout << "macie polane " << std::endl;
                hostCount--;
                break;
            case REQALC:
                message = ACKALC;
                sendMessage(rank, message, msg.id, lamport);
                break;
            case ACKALC:
                std::cout << "masz alko" << std::endl;
                alcohol = 1;
                break;
            case PARTYHARD:
                deleteHostGroup(msg.id);
                if (msg.id == myHost) {
                    //std::cout << rank << " Odebralem PARTYHARD\n";
                    partyStarted = true;
                }
                break;
            case PARTYOVER:
            std::cout << rank << " zachlalem sie, ide na domek peeposhy\n";
                partyOver = true;
                break;
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
                incrementLamport();
                if (perc > 25) {
                    std::cout << rank << " i IMPREZA" << std::endl;
                    incrementLamport();
                    ackCount = size - 1;
                    message_t message = REQPARTY;
                    Message temp;
                    temp.id = rank;
                    temp.lamport = lamport;
                    insertQueue(temp);
                    std::cout << rank << " WSTAWIAM SIEBIE\n";
                    printPartyQueue();
                    for (int i = 0; i < size; i++) {
                        if (i != rank) {
                            const int sendId = i;
                            //std::cout << rank << ": wysylam powiadomienie do: " << sendId << std::endl;
                            sendMessage(rank, message, sendId, temp.lamport);
                        }
                    }
                    while (ackCount > 0) {}
                    
                    //std::cout << rank << ": dostalem potwierdzenie od wszystkich POG" << std::endl;
                    changeState(WAITHOST);
                }
                else {
                    std::cout << "Jestem " << (rank < BUNNY ? "zajacem " : "misiem ") << rank << " i wylosowalem liczbe: " << perc << std::endl;
                    sleep(2);
                }
                break;
            case WAITHOST:
                //std::cout << rank << ": dostalem potwierdzenie od wszystkich POG" << std::endl;
                
                if (partyQueue.begin()->id == rank) {
                    changeState(WAITGROUP);
                    //printPartyQueue();
                }             
                else {
                    
                    while (!invited) { 
                        //sleep(1);
                        //std::cout << rank << " jestwem w WAITHOST\n";
                    }
                    updateQueue();
                    if (myHost != -1) {
                        changeState(WAITMEADOW);
                    } else {
                        invited = false;
                    }
                }
                break;
            case WAITGROUP: {
                //sleep(1);
                //std::cout << rank << " JESTEM GOSPODARZEEM"<< std::endl;
                int inviteCount = 0;
                int isBunny = 0;
                inviteList.clear();
                // std::cout << rank << " Wyczyszczona kolejka zaproszonych: \n";
                // for (Message animal : inviteList) {
                //     std::cout << animal.id<< " ";
                // }
                if (rank < BUNNY) {
                    isBunny = 1;
                }
                for (const Message animal : partyQueue) {
                    if (!isBunny && MEADOWSIZE - inviteCount <= 4 && animal.id >= BUNNY) {
                        continue;
                    }
                    if (inviteCount + (animal.id < BUNNY ? 1 : 4) <= MEADOWSIZE) {
                        std::cout << rank << ": zapraszam = " << animal.id << std::endl;
                        inviteCount += (animal.id < BUNNY ? 1 : 4);
                        inviteList.push_back(animal);
                    }
                    if (inviteCount == MEADOWSIZE) { //found enough animals to form a party
                        std::cout << rank << " Moja kolejka zaproszonych: \n";
                            for (Message animal : inviteList) {
                                std::cout << animal.id<< " ";
                            }
                        std::cout << std::endl;
                        for (int i = 0; i < size; i++) {
                            if (i != rank) {
                                bool found = false;

                                for (Message animal : inviteList) {
                                    if (animal.id == i) {
                                        message_t message = TAKINGYOU; //informing invited animals that they will be having a party
                                        sendMessage(rank, message, i, lamport);
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) {
                                    message_t message = TAKINGTHEM; //informing the rest of animals to remove invited ones from the list
                                    sendMessage(rank, message, i, lamport);
                                }
                            }
                        }
                        std::cout << rank << " Jako gospodarz mam do dyspozycji kolejke: \n";
                        printPartyQueue();
                        updateQueue();
                        std::cout << rank << " Jako gospodarz zaprosilem ludzi, pozostali: \n";
                        printPartyQueue();
                        Message temp;
                        temp.id = rank;
                        temp.lamport = lamport;
                        hostGroup.insert(temp);
                        changeState(WAITMEADOWHOST);
                        break;
                    }
                    if (inviteCount != MEADOWSIZE) {
                        changeState(WAITHOST);
                    }
                }
                break;
            }
 
            case WAITMEADOW:
               // std::cout << rank << " !!!!!!!!!!!!!!Czekam az gospodarz zrobim party wooo\n";
                incrementLamport();
                while (!partyStarted) {
                    //sleep(1);
                    //std::cout << rank << " Czekam na impre\n";
                }
                changeState(PARTY);
                break;
            case WAITMEADOWHOST: {
                //std::cout << rank << " !!!!!!!!!!!!!!!Czekam az dostane polane i bedzie party wooo\n";
                sleep(1);
                printHostGroup();

                if (hostGroup.begin()->id == rank || hostGroup.size() == 1) {
                    
                    hostCount = hostGroup.size() - 1;
                    for (Message host : hostGroup) {
                        if (host.id != rank) {
                            message_t message = REQMEADOW;
                            sendMessage(rank, message, host.id, lamport);
                        }
                    }
                    while (hostCount > 0) {} //wait for others to accept meadow request
                    while (partyMeadow == -1) {   
                        partyMeadow = takeMeadow(); //TODO: eliminate mutex fighting
                    }
                    
                    for (Message host : hostGroup) {
                        if (host.id != rank) {
                            message_t message = PARTYHARD;
                            sendMessage(rank, message, host.id, lamport);
                        }
                    }
                    deleteHostGroup(rank);
                    bool isBear = false;
                    for (Message animal : inviteList) {
                        if (animal.id >= BUNNY) {
                            isBear = true;
                            break;
                        }
                    }
                    if (isBear) {
                        for (Message animal : inviteList) {
                            if (animal.id < BUNNY) {
                                alcohol = 0;
                                message_t message = REQALC;
                                sendMessage(rank, message, animal.id, lamport);
                                while (alcohol == 0) {}
                                break;
                            }
                        }
                        if (alcohol == 0) {
                            printf("[%d,%d]I guess jestem jedynym bunny na imprezie :c\n", rank, lamport);
                        }
                    }
                    for (int i = 0; i < size; i++) {
                        if (i != rank) {
                            //std::cout << "wysylam zaproszenie na impreze do: " << i << std::endl;
                            message_t message = PARTYHARD;
                            sendMessage(rank, message, i, lamport);
                        }
                    }
                    
                    sleep(3);
                    for (Message animal : inviteList) {
                        if (animal.id != rank) {
                            message_t message = PARTYOVER;
                            sendMessage(rank, message, animal.id, lamport);
                        }
                    }
                    releaseMeadow(partyMeadow);
                    partyMeadow = -1;
                    inviteList.clear();
                    changeState(REST);
                }
                break;
            }
            case PARTY:
                printf("[%d,%d] Disco disco Party party\n", rank, lamport);
                while (!partyOver) {}
                changeState(REST);
                partyOver = false;
                partyStarted = false;
                changeInvited();
                myHost = -1;
                break;

            default:
                std::cout << rank << " I'm in backrooms\n";
        }
    }
}