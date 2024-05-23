#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

/* DEBUG: SARANNO DA TOGLIERE I COMMENTI DA QUESTE E DA CANCELLARE LE LIBRERIE SOTTO, OLTRE A CANCELLARE LA CARTELLA LIBRERIE */
/*#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"
#include "/usr/include/umps3/umps/aout.h"
#include "/usr/include/umps3/umps/arch.h"
#include "/usr/include/umps3/umps/bios_defs.h"
#include "/usr/include/umps3/umps/types.h"*/
#include "./../../librerie/libumps.h"
#include "./../../librerie/const.h"
#include "./../../librerie/const.h"
#include "./../../librerie/cp0.h"
#include "./../../librerie/aout.h"
#include "./../../librerie/arch.h"
#include "./../../librerie/bios_defs.h"
#include "./../../librerie/types.h"

#include "../../headers/types.h"
#include "../../headers/const.h"
#include "../../phase1/headers/pcb.h"
#include "../../phase1/headers/msg.h"

#include "./scheduler.h"
#include "./exceptions.h"
#include "./interrupts.h"
#include "./ssi.h"

extern void test ();
extern void scheduler ();
extern void uTLB_RefillHandler ();
extern void exceptionHandler ();
extern void remoteProcedureCall();

#endif