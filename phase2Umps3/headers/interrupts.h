#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include "./initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
extern cpu_t ultimo;

#define EXCEPTION_STATE = (state_t *)BIOSDATAPAGE;

int intconst[7];

void interrupthandler();//funzioni abbastanza ovvie comunque sotto spiego cosa fanno
void IT_handler();
void NT_handler(int);//forse qui va qualcosa in input
void startinterrupt();
void endinterrupt();
void get_deviceinterrupt(int *);
int get_numdevice(int);
int get_status(int);
void set_status(char);
cpu_t tempopassato();

#endif