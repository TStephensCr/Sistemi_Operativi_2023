#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "./initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
extern cpu_t ultimo;

void scheduler();

#endif