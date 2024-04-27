#ifndef ANIMAL_H
#define ANIMAL_H

void *mainLoop(void *ptr);

typedef enum {REST, WAITHOST, WAITGROUP, WAITMEADOW, WAITMEADOWHOST, PARTY} state_t;
extern state_t state;

#endif