#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "./initial.h"

extern struct list_head PseudoClockWP;

/*
DA ELIMINARE:
    void terminateProcess(pcb_t* process);
    void terminateProcessTree(pcb_t* process);
    void findDeviceNum(memaddr commandAddr, pcb_t *p, unsigned int *device_num, unsigned int *device_line);
    void remoteProcedureCall();
    void SSIRequest(pcb_t* sender, int service, void* ar);
*/
extern void copyState();

void remoteProcedureCall();

#endif