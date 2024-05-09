#include "/usr/include/umps3/umps/libumps.h"
#include "./headers/ssi.h"

static void terminateProcess(pcb_t* process){

    outChild(process);

    //If a terminated process is waiting for the completion of a DoIO, the value used to track this should be adjusted accordingly. 
    int blocked = FALSE;

    for(int i = 0; i < SEMDEVLEN; i++){
        for(int j = 0; j < 2; j++){

            if(blockedpcbs[i][j] == (pcb_PTR)process){

                blockedpcbs[i][j] = NULL;

                blocked = TRUE;

                break;
            }

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

static void findDeviceNum(memaddr commandAddr, pcb_t *p, unsigned int *device_num, unsigned int *device_line){//dubbio: forse cambiare & in * e aggiungere & dove la funzione viene chiamata
    for (int i = 3; i < 8; i++){
        for (int k = 0; k < 8; k++){ 

            dtpreg_t *baseAddr = (dtpreg_t *)DEV_REG_ADDR(i, k);//base address for device

            if(commandAddr == (memaddr)(baseAddr->command) ){//forse serve & su command

                device_num = (unsigned int*)k;
                device_line = (unsigned int*)i;

                return;
            }else if(i == 7 && commandAddr == (memaddr)(baseAddr->command)){

                device_num = (unsigned int*)k;
                device_line = (unsigned int*)i;
                
                return;
            }

        }  
    }
}

static void SSIRequest(pcb_t* sender, int service, void* ar){
    switch(service){
        case 1:
        //CreateProcess
            //check resources availability
            if(emptyProcQ(&pcbFree_h)){
                ar = (void*)NOPROC;
            }

            //initialize new process

            pcb_t *newProcess = allocPcb();

            ssi_create_process_t *args = (ssi_create_process_t*)ar;//save struct passed as argument

            copyState(newProcess->p_s, *args->state);   //p_s from arg->state.

            newProcess->p_supportStruct = args->support;//p_supportStruct from arg->support. If no parameter is provided, this field is set to NULL.
            
            newProcess->p_time = 0;//p_time is set to zero; the new process has yet to accumulate any CPU time
            
            insertProcQ(&sender->p_list, newProcess);//The process queue field (e.g. p_list) by the call to insertProcQ
            
            insertChild(sender, newProcess);//The process tree field (e.g. p_sib) by the call to insertChild.
            
            LDST(&current_process->p_s);//return control to the current process
            break;
        case 3:
        //DOIO

            ssi_do_io_t *do_io = (ssi_do_io_t *)ar;//do_io struct passed as argument

            unsigned int *commandAddr = do_io->commandAddr;

            unsigned int commandValue = do_io->commandValue;

            unsigned int device_num;

            unsigned int device_line;

            findDeviceNum((memaddr)commandAddr, sender, &device_num, &device_line);//finding device number and saving it to sender pcb

            *commandAddr = commandValue;//the SSI will write the requested value on the device

            int devIndex = EXT_IL_INDEX(device_line) * N_DEV_PER_IL + device_num;

            //ATTENZIONE NON SI SA ANCORA PERCHE' IL SECONDO INDEX E' 0 E NON 1, NON SI SA QUAL'E' GIUSTO
            blockedpcbs[devIndex][0] = sender;//the process will wait for a response from the SSI

            softBlockCount++;

            break;
        case 4:
        //GETCPUTIME

            ar = &(sender->p_time);

            break;
        case 5:
        //WaitForClock

            softBlockCount++;

            insertProcQ(&PseudoClockWP, sender);

            break;
        case 6:
        //GetSupportData

            ar = sender->p_supportStruct;

            break;
        case 7:
        //GetProcessID

            if(ar == 0){//return sender's PID if argument is 0
                ar = &(sender->p_pid);
                break;
            }

            if(sender->p_parent != NULL)//return sender's parent's PID otherwise
                ar = &(sender->p_parent->p_pid);

            else ar = 0;//return 0 if sender is root and argument is not 0

            break;
        default:
        //Terminate Process

            ar = (service) ? NULL : ar; //If service is null, the sender process must be terminated, regardless of the argument

            pcb_t *tmp_pcb = (ar == NULL) ? sender : ar;//If the argument is null, the sender process must be terminated

            terminateProcessTree(tmp_pcb);

            break;
    }
}

void remoteProcedureCall(){
    while(TRUE){
        ssi_payload_t payload;

        //receive request
        int sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        //satisfy request
        SSIRequest((pcb_t*)sender, payload.service_code, payload.arg);//dubbio: forse vanno create variabili invece che passare direttamente 

        //send back results
        SYSCALL(SENDMESSAGE, sender, (unsigned int)(payload.arg), 0);
    }
}
