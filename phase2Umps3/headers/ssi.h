#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "./initial.h"

void terminateProcess(pcb_t* process);

void terminateProcessTree(pcb_t* process);

void findDeviceNum(memaddr commandAddr, pcb_t *p, unsigned int *device_num, unsigned int *device_line);

void SSIRequest(pcb_t* sender, int service, void* ar);

void remoteProcedureCall();

#endif