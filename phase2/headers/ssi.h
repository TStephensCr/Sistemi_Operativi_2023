#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "./initial.h"

extern struct list_head PseudoClockWP;

extern void copyState();

void remoteProcedureCall();

#endif