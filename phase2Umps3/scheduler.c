#include "./headers/scheduler.h"

void scheduler(){

    if(emptyProcQ(&readyQueue) == 0){
        //  1. Se c'e' almeno un processo pronto ad essere eseguito, lo prendo dalla Ready e lo assegno al current_process.
        current_process = removeProcQ(&readyQueue);
        setTIMER(TIMESLICE);
        STCK(start);
        LDST(&(current_process->p_s));//perform a Load Processor State on the current_process's processor state
    }
    //only ssi is running
    if(process_count == 1) HALT();

    if(process_count > 1 && softBlockCount > 0){//enter a WAIT state
        setSTATUS ((getSTATUS () | IECON | IMON) & ~TEBITON);
        WAIT();
    } 

    if(process_count > 0 && softBlockCount == 0) PANIC(); //DeadLock state
}