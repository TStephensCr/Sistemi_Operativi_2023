#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include "./initial.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
extern pcb_t ultimo;



int intconst[7] = { 0x00000001,//device 0
                    0x00000002,//device 1
                    0x00000004,//device 2
                    0x00000008,//device 3
                    0x00000010,//device 4
                    0x00000020,//device 5
                    0x00000040,//device 6
                    0x00000080};//device 7

void interrupthandler();//funzioni abbastanza ovvie comunque sotto spiego cosa fanno
void NT_handler(int);//forse qui va qualcosa in input
void startinterrupt();
void endinterrupt();
void get_deviceinterrupt(int *);
int get_numdevice(int);
int get_status(int);
void set_status(char);
cpu_t tempopassato();
static sbloccapcb(int,int, pcb_PTR);

#endif