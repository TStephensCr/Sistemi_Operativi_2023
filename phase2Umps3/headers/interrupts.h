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

/*
DA ELIMINARE:
    void sbloccapcb(int,int, pcb_PTR blockedpcbs[SEMDEVLEN]);
    void removeBlocked(pcb_t *pcb, pcb_PTR blockedpcbs[SEMDEVLEN]);
    cpu_t tempopassato();
    void startinterrupt();
    void endinterrupt();
    void NT_handler(int);//forse qui va qualcosa in input
    int get_numdevice(int);
    void get_deviceinterrupt(int *);
    int get_status(int);
    void set_status(char);
*/

#endif