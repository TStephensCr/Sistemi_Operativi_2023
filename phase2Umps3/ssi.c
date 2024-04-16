#include "/usr/include/umps3/umps/libumps.h"
#include "./headers/ssi.h"

void SSIRequest(pcb_t* sender, int service, void* ar){
    switch(service){
        case 1:
            //check resources availability
            if(emptyProcQ(&freePcb)){
                return NOPROC;
            }

            //initialize new process

            pcb_t *newProcess = allocPcb();
            newProcess->p_s = *ar->state;
            newProcess->p_supportStruct = *ar->support;
            newProcess->p_time = 0;
            insertProcQ(&sender->p_list, newProcess);
            insertChild(sender, newProcess);
            //return control current process
            LDST(&currentProcess->p_s)
            break;
        case 3:
            //DOIO
            break;
        case 4:
            return sender->p_time;
            break;
        case 5:
            //put sender in WAIT until next pseudo-clock tick, and keep a list of those waiting
            break;
        case 6:
            return sender->p_supportStruct;
            break;
        case 7:
            if(ar == 0)
                return sender->p_pid;
            if(sender->p_parent != NULL)
                return sender->p_parent->p_pid;
            return 0;
            break;
        default:
            ar = (service == NULL) ? NULL : ar; //If service is null, the sender process must be terminated, regardless of the argument
            pcb_t tmp_pcb = (ar == NULL) ? sender : ar;//If the argument is null, the sender process must be terminated

            do{
                terminateProcess(tmp_pcb);
                tmp_pcb = tmp_pcb->
            }while()

            break;
    }
}

void remoteProcedureCall(){
    while(TRUE){
    //receive request
    SSIRequest(pcb_t* sender, int service, void* arg);//satisfy request
    //send back results
    }
}

void terminateProcess(pcb_t* process){

    outChild(process);

    //If a terminated process is waiting for the completion of a DoIO, the value used to track this should be adjusted accordingly.

    //If a terminated process is waiting for clock, the value used to track this should be adjusted accordingly.

    processCount--;

    //Se sto processo sta aspettando IO o il clock allora softBlockCount--;

}

