#include "./headers/exceptions.h"

//DEBUG: mancano 5.4 e 5.5 ma lo faccio direttamente quando facciamo debug che devo capire delle cose prima

void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t*) 0x0FFFF000);
}


// aggiorna p_time del processo p
void saveTime(pcb_t *p) {
    debug("\nu",2,0);
    int end;
    STCK(end);
    p->p_time += (end - start);
    start = end;
}


static void die(pcb_PTR p){
    while(emptyChild(p)==0){
        pcb_PTR son = removeChild(p);
        if(son==NULL) die(removeChild(p));
    }
    freePcb(p);
    process_count--;
}

/*
Section 9
"Pass Up or Die"
*/
static void kill(int index){
//Die
    if(current_process->p_supportStruct==NULL){
        outChild(current_process);               //Lo tolgo dai puntatori ai figli del padre 
        die(current_process);
        current_process=NULL;
        scheduler();
    }else{
//Pass Up
        copyState(&(current_process->p_supportStruct->sup_exceptState[index]), (state_t*)BIOSDATAPAGE);
        //Sezione 7.3.4 del manuale: (u-int stackPtr,u-int status,u-int pc)
        LDCXT(current_process->p_supportStruct->sup_exceptContext[index].stackPtr,
            current_process->p_supportStruct->sup_exceptContext[index].status,
            current_process->p_supportStruct->sup_exceptContext[index].pc);
    }
}

int sendMsg(pcb_t *senderAddr, pcb_t *destinationAddr, unsigned int payload) {
    msg_t *msg = allocMsg();
    if(msg==NULL) return MSGNOGOOD;  //non ho più messaggi disponibili
    
    msg->m_sender = senderAddr;
    msg->m_payload = payload;
    insertMessage(&(destinationAddr->msg_inbox), msg);
    return 0;
}

static int SYS1_sendMessage(state_t* excState){
    pcb_PTR destinationAddr = (pcb_PTR)excState->reg_a1;
    unsigned int payload = (unsigned int)excState->reg_a2;

    if(pcbIsInList(destinationAddr,&pcbFree_h)==1) return DEST_NOT_EXIST;

    //Faccio la send, se ha successo, il pcb destinatario va svegliato mettendolo nella readyQueue
    if(sendMsg(current_process, destinationAddr, payload))
        if(pcbIsInList(destinationAddr,&readyQueue)!=1)
            insertProcQ(&readyQueue, destinationAddr);
    return 0;
} 

static int SYS2_receiveMessage(state_t* excState){
    pcb_PTR senderAddr = (pcb_PTR)excState->reg_a1;
    memaddr* whereToSave = (memaddr*)excState->reg_a2;

    msg_PTR msg;

    if(senderAddr == (pcb_PTR)ANYMESSAGE){ //prendiamo il primo messaggio nella lista
        msg = popMessage(&(current_process->msg_inbox),NULL);
    }else{
        msg = popMessage(&(current_process->msg_inbox),senderAddr);
    }
    if(!msg) {  //paragrafo 5.5
        copyState(&(current_process->p_s), excState);
        saveTime(current_process);
        scheduler();  
    }

    if(msg->m_payload != (memaddr)NULL) *whereToSave = msg->m_payload;
    freeMsg(msg);

    return (memaddr)msg->m_sender;
}

/*
Gestisce le eccezioni, capisce che tipo di eccezione è, quindi passa la palla ad un'altra funzione, in base al numero dell'excCode
Section 4
*/
void exceptionHandler(){
    state_t *excState = (state_t *)BIOSDATAPAGE;       //Stato dell'eccezione salvata all'inizio di BIOSDATAPAGE
    unsigned int excCode = (getCAUSE() & GETEXECCODE) >> CAUSESHIFT;     //Codice (motivo) dell'eccezione

    debug("\ne",2,excCode);
    switch(excCode){
        //Interrupts
        case IOINTERRUPTS:  //0                                            //DEBUG: fatto (credo)
            debug("\ni",2,4);
            interrupthandler();
            break;

        //TLB Exceptions                                                    //DEBUG: fatto (credo)
        case 1:
        case TLBINVLDL:     //2
        case TLBINVLDS:     //3
            kill(PGFAULTEXCEPT);
            break;

        //Program Traps                                                     //DEBUG: fatto (credo)
        case 4: case 5: case 6: case 7:
        case BREAKEXCEPTION: case PRIVINSTR:    //9,10
        case 11: case 12:
            kill(GENERALEXCEPT);
            break;

        //SYSCALL
        case SYSEXCEPTION:              //8                                 //DEBUG: manca questo
            //SYSCALL fatta in user-mode
            if((excState->status & USERPON) != 0){
		        setCAUSE(PRIVINSTR);
                exceptionHandler();
	        }
            //SYSCALL fatta in kernel-mode
            int a0 = excState->reg_a0;
            if(a0 == SENDMESSAGE)           //a0 = -1
                excState->reg_v0 = SYS1_sendMessage(excState);
            else if(a0 == RECEIVEMESSAGE)   {//a0 = -2
                excState->reg_v0 = SYS2_receiveMessage(excState);        //v0
             } else kill(GENERALEXCEPT);        //a0 != -1, -2  (Sezione 9.1)
            break;
    }
}