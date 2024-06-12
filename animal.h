#ifndef ANIMAL_H
#define ANIMAL_H
 
extern int lamport;
extern int ackCount;
 
void *mainLoop(void *ptr);
 
typedef enum {REST, WAITHOST, WAITGROUP, WAITMEADOW, WAITMEADOWHOST, PARTY} state_t;
extern state_t state;
 
typedef enum {REQPARTY, ACKPARTY, REQMEADOW, ACKMEADOW, REQALC, TAKINGYOU, TAKINGTHEM, PARTYHARD, PARTYOVER} message_t;
extern message_t message;
 
typedef struct {
    int id;
    int lamport;
} Message;
 
extern std::vector<Message> partyQueue;
extern std::mutex mutexQueue;
 
#endif