#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "./initial.h"

void SSIRequest(pcb_t* sender, int service, void* ar);

void remoteProcedureCall();

#endif