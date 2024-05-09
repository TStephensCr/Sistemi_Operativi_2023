#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED
#include "initial.h"

extern int process_count;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR current_process;
extern pcb_PTR blockedpcbs[SEMDEVLEN-1][2];
extern cpu_t ultimo;

void exceptionHandler();
void uTLB_RefillHandler();

extern struct list_head pcbFree_h;
extern int sendMsg();

#endif