#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED
#include "initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN-1][2];
extern cpu_t ultimo;

extern struct list_head pcbFree_h;

void saveState(state_t* new_state, state_t* old_state);
void exceptionHandler();
void uTLB_RefillHandler();

#endif