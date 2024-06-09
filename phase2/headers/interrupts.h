#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include "./initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN-1];
extern int ultimo;

void interrupthandler();//funzioni abbastanza ovvie comunque sotto spiego cosa fanno
void copyState();

extern int sendMsg();

#endif