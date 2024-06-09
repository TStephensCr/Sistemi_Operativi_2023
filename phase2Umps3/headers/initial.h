#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"
#include "/usr/include/umps3/umps/aout.h"
#include "/usr/include/umps3/umps/arch.h"
#include "/usr/include/umps3/umps/bios_defs.h"
#include "/usr/include/umps3/umps/types.h"

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"

#include "./scheduler.h"
#include "./exceptions.h"
#include "./interrupts.h"
#include "./ssi.h"
#include "./print.h"

extern void test ();
extern void scheduler ();
extern void uTLB_RefillHandler ();
extern void exceptionHandler ();
extern void remoteProcedureCall();

#endif