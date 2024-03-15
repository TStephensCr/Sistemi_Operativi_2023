#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include "./scheduler.h"
#include "./utile.h"
#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../headers/const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"

#define EXCEPTION_STATE = (state_t *)BIOSDATAPAGE;
unsigned int processCount;                    //Process Count, numero di processi attivi e non terminati
unsigned int softBlockCount;                   //Soft-Block Count, numero di processi in waiting per I/O o per tempo esaurito
struct list_head readyQueue;                //Ready Queue, puntatore alla coda di porcessi in ready
pcb_PTR currentProcess;                          //Current Process, punta al processo in running
pcb_PTR blockedpcbs[SEMDEVLEN][2];                   //idk va capito
cpu_t ultimo;//                                         ultimo TOD

void second_pcb();
void first_pcb();
int main();

#endif
