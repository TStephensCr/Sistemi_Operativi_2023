#include "./headers/scheduler.h"

void scheduler(){
    current_process = removeProcQ(&readyQueue);
    if(current_process != NULL){
        current_process->p_s.status = current_process->p_s.status | TEBITON;
        setTIMER(TIMESLICE);
        LDST(&current_process->p_s);
    }
    if(process_count == 1){
        HALT();
    }
    if(process_count > 1 && softBlockCount > 0){
        setSTATUS ((getSTATUS () | IECON | IMON) & ~TEBITON);
        WAIT();
    } 
}