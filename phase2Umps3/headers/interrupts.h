#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include "./initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN-1][2];
extern int ultimo;

int intconst[8] = { 0x00000001,//device 0
                    0x00000002,//device 1
                    0x00000004,//device 2
                    0x00000008,//device 3
                    0x00000010,//device 4
                    0x00000020,//device 5
                    0x00000040,//device 6
                    0x00000080};//device 7

void interrupthandler();//funzioni abbastanza ovvie comunque sotto spiego cosa fanno
/*
DA ELIMINARE:
    void sbloccapcb(int,int, pcb_PTR blockedpcbs[SEMDEVLEN][2]);
    void removeBlocked(pcb_t *pcb, pcb_PTR blockedpcbs[SEMDEVLEN][2]);
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