#include "/usr/include/umps3/umps/libumps.h"
#include "initial.c"

void scheduler(){
    currentProcess = removeProcQ(&readyQueue);
    if(currentProcess != NULL){
        currentProcess->p_s.status = currentProcess->p_s.status | TEBITON;
        setTIMER(TIMESLICE);
        LDST(&currentProcess->p_s);
    }
    if(processCount == 1){
        HALT();
    }
    if(processCount > 1 && softBlockCount > 0){
        setSTATUS ((getSTATUS () | IECON | IMON) & ~TEBITON);
        WAIT();
    } 
}