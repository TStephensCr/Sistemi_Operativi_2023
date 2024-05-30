#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "./initial.h"

extern int process_count;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR current_process;
extern pcb_PTR blockedpcbs[SEMDEVLEN-1];
extern cpu_t ultimo;
extern int start;

void scheduler();

#endif