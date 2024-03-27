#include "/usr/include/umps3/umps/libumps.h"

void SSIRequest(pcb_t* sender, int service, void* ar){
    switch(service){
        case 1:
            createProcess();
            break;
        case 2:
            terminateProcess((pcb_t*)ar);
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
        case 7;
            if(ar == 0)
                return sender->p_pid;
            if(sender->p_parent != NULL)
                return sender->p_parent->p_pid;
            return 0;
    }
}

void remoteProcedureCall(){
    while(TRUE){
    //receive request
    SSIRequest(pcb_t* sender, int service, void* arg);//satisfy request
    //send back results
    }
}

pcb_t* createProcess(){

}

void terminateProcess(pcb_t* process){

}

