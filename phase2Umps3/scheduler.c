#include "./headers/scheduler.h"

void scheduler(){

    current_process = removeProcQ(&readyQueue);//Remove the PCB from the head of the Ready Queue

    if(current_process != NULL){
        //current_process->p_s.status = current_process->p_s.status | TEBITON;//store the pcb in the current_process field
        setTIMER(TIMESLICE);//load 5 milliseconds on the PLT

        STCK(start);
        LDST(&(current_process->p_s));//perform a Load Processor State on the current_process's processor state
    }

    if(process_count == 1){ HALT(); }//only ssi is running

    if(process_count > 1 && softBlockCount > 0){//enter a WAIT state

        setSTATUS ((getSTATUS () | IECON | IMON) & ~TEBITON);

        WAIT();

    } 

    if(process_count > 0 && softBlockCount == 0){ PANIC(); }//DeadLock state
}