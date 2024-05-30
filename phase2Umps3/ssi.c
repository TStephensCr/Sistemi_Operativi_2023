#include "./headers/ssi.h"

static void terminateProcess(pcb_t* process){

    outChild(process);

    //If a terminated process is waiting for the completion of a DoIO, the value used to track this should be adjusted accordingly. 
    int blocked = FALSE;

    for(int i = 0; i < SEMDEVLEN; i++){
        if(blockedpcbs[i] == (pcb_PTR)process){

            blockedpcbs[i] = NULL;

            blocked = TRUE;

            break;
        }
    }

    //If a terminated process is waiting for clock, the value used to track this should be adjusted accordingly.
    if(outProcQ(&PseudoClockWP, process))
        blocked = TRUE;

    process_count--;

    if(blocked) softBlockCount--;

}

static void terminateProcessTree(pcb_t *process) {
    if (process == NULL)
        return;

    // Terminate the current process
    terminateProcess(process);

    // Traverse through children
    pcb_t *child = headProcQ(&process->p_child);//non gli va bene questo argomento

    while (child != NULL) {
        // Recursion call on children
        terminateProcessTree(child);

        child = headProcQ(&child->p_sib); // Call for each sibling
    }
}

static void findDeviceNum(memaddr commandAddr, unsigned int *device_num, unsigned int *device_line){
    //cerco prima tra i terminali (gli unici dispositivi che sono presenti in questa fase)
    for(int i=0; i<16; i+=2){
        termreg_t* termReg = (termreg_t*)DEV_REG_ADDR(7, i);    //Macro che trova l'indirizzo del device data la line e il num, definita in arch.h
        if(commandAddr == (memaddr)&(termReg->transm_command)){   //controllo il sub-channel di scrittura
            *device_line = 7;   //linea dei terminali
            *device_num = i;
            return;
        }if(commandAddr == (memaddr)&(termReg->recv_command)){   //controllo il sub-channel di lettura
            *device_line = 7;   //linea dei terminali
            *device_num = i+1;
            return;
        }
    }
    //cerco tra ognuno degli 8 canali dei 4 dispositivi (da 3 a 6) dei non-terminali
    for(int i=3; i<7; i++){
        for(int j=0; j<8; j++){
            dtpreg_t* devReg = (dtpreg_t*)DEV_REG_ADDR(i, j);    //Macro che trova l'indirizzo del device data la line e il num, definita in arch.h
            if(commandAddr == (memaddr)&(devReg->command)){   //vedo se l'indirizzo di base_address->command con command_address
                *device_line = i;
                *device_num = j;
                return;
            }
        }
    }
}

static unsigned int SSIRequest(pcb_t* sender, ssi_payload_t* payload){
    int ris=0;  //DEBUG: da togliere da qui e da applicare ad ogni case
    void* ar = payload->arg;
    int service = payload->service_code;
    switch(payload->service_code){
        case CREATEPROCESS: //1
            adebug0();
            //check resources availability
            if(emptyProcQ(&pcbFree_h)) return NOPROC;

            //initialize new process
            pcb_PTR newProcess = allocPcb();
            newProcess->p_time = 0;//p_time is set to zero; the new process has yet to accumulate any CPU time
            newProcess->p_supportStruct = ((ssi_create_process_PTR)payload->arg)->support;//p_supportStruct from arg->support. If no parameter is provided, this field is set to NULL.
            copyState(&(newProcess->p_s), ((ssi_create_process_PTR)payload->arg)->state);   //p_s from arg->state.s
            
            //si_create_process_t *args = (ssi_create_process_t*)ar;//save struct passed as argument            
            
            insertChild(sender, newProcess);//The process tree field (e.g. p_sib) by the call to insertChild.

            insertProcQ(&readyQueue, newProcess);//The process queue field (e.g. p_list) by the call to insertProcQ
            
            process_count++;
            ris=(unsigned int)newProcess;
            //LDST(&current_process->p_s);//return control to the current process
            break;
        case TERMPROCESS:   //2
            adebug6();
            ar = (service) ? NULL : ar; //If service is null, the sender process must be terminated, regardless of the argument

            pcb_t *tmp_pcb = (ar == NULL) ? sender : ar;//If the argument is null, the sender process must be terminated

            terminateProcessTree(tmp_pcb);

            break;
        case DOIO:  //3
            adebug1();

            //do_io struct passed as argument
            ssi_do_io_t* do_io = (ssi_do_io_t*)payload->arg;

            unsigned int commandValue = do_io->commandValue;

            unsigned int device_num = -1;
            unsigned int device_line = -1;
            findDeviceNum((memaddr)do_io->commandAddr, &device_num, &device_line);//finding device number and saving it to sender pcb

            //if(device_num == -1 && device_line==-1) PANIC();    //DEBUG:secondo me avrebbe senso
            
            *(do_io->commandAddr) = commandValue;//the SSI will write the requested value on the device

            //devIndex = (line-3)*8 + num
            int devIndex = EXT_IL_INDEX(device_line) * N_DEV_PER_IL + device_num;   

            outProcQ(&readyQueue, sender);  //tolgo dai ready se era lì
            blockedpcbs[devIndex] = sender; //the process will wait for a response from the SSI

            softBlockCount++;
            ris=-1;
            break;
        case GETTIME:   //4
            adebug2();
            ar = &(sender->p_time);

            break;
        case CLOCKWAIT: //5
            adebug3();
            softBlockCount++;

            insertProcQ(&PseudoClockWP, sender);

            break;
        case GETSUPPORTPTR: //6
            adebug4();
            ar = sender->p_supportStruct;

            break;
        case GETPROCESSID:  //7
            adebug5();

            if(ar == 0){//return sender's PID if argument is 0
                ar = &(sender->p_pid);
                break;
            }

            if(sender->p_parent != NULL)//return sender's parent's PID otherwise
                ar = &(sender->p_parent->p_pid);

            else ar = 0;//return 0 if sender is root and argument is not 0

            break;
    }
    return ris;
}

void remoteProcedureCall(){
    while(TRUE){
        ssi_payload_t* payload;

        //receive request
        unsigned int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        //satisfy request
        unsigned int ris = SSIRequest((pcb_PTR)sender, payload);

        //send back results
        if(ris!=-1) SYSCALL(SENDMESSAGE, sender, ris, 0);
    }
}
