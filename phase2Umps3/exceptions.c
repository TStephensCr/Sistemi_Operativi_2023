#include "./headers/exceptions.h"

//DEBUG: mancano 5.4 e 5.5 ma lo faccio direttamente quando facciamo debug che devo capire delle cose prima

void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t*) 0x0FFFF000);
}

//salvataggio dello stato
void saveState(state_t* dest, state_t* to_copy) {
    //copia di tutti i parametri di uno stato nell'altro
    dest->entry_hi = to_copy->entry_hi;
    dest->cause = to_copy->cause;
    dest->status = to_copy->status;
    dest->pc_epc = to_copy->pc_epc;
    for(int i = 0; i < STATE_GPR_LEN; i++) 
        dest->gpr[i] = to_copy->gpr[i];
    dest->hi = to_copy->hi;
    dest->lo = to_copy->lo;
}

static void die(pcb_PTR p){
    while(emptyChild(p)==0){
        pcb_PTR son = removeChild(p);
        if(son==NULL) die(removeChild(p));
    }
    freePcb(p);
    processCount--;
}

/*
Section 9
"Pass Up or Die"
*/
static void kill(int index){
//Die
    if(currentProcess->p_supportStruct==NULL){
        outChild(currentProcess);               //Lo tolgo dai puntatori ai figli del padre 
        die(currentProcess);
        currentProcess=NULL;
        scheduler();
    }else{
//Pass Up
        currentProcess->p_supportStruct->sup_exceptState[index] = *(state_t*)BIOSDATAPAGE;
        //Sezione 7.3.4 del manuale: (u-int stackPtr,u-int status,u-int pc)
        LDCXT(currentProcess->p_supportStruct->sup_exceptContext[index].stackPtr,
            currentProcess->p_supportStruct->sup_exceptContext[index].status,
            currentProcess->p_supportStruct->sup_exceptContext[index].pc);
    }
}

static int SYS1_sendMessage(){
    pcb_PTR destinationAddr = (pcb_PTR)currentProcess->p_s.reg_a1;
    unsigned int payload = (unsigned int)currentProcess->p_s.reg_a2;

    if(pcbIsInList(destinationAddr,&pcbFree_h)==1) return DEST_NOT_EXIST;

    msg_t *msg = allocMsg();
    if(msg==NULL) return MSGNOGOOD;  //non ho più messaggi disponibili

    //il pcb destinatario va svegliato mettendolo nella readyQueue
    if(pcbIsInList(destinationAddr,&readyQueue)!=1) insertProcQ(&readyQueue, destinationAddr);
    
    msg->m_sender = currentProcess;
    msg->m_payload = payload;
    insertMessage(&(destinationAddr->msg_inbox), msg);
    return 0;
} 


static int SYS2_receiveMessage(){
    pcb_PTR senderAddr = (pcb_PTR)currentProcess->p_s.reg_a1;
    memaddr* whereToSave = (memaddr*)currentProcess->p_s.reg_a2;

    msg_PTR msg;

    if(senderAddr == (pcb_PTR)ANYMESSAGE){ //prendiamo il primo messaggio nella lista
        msg = popMessage(&(currentProcess->msg_inbox),NULL);
    }else{
        msg = popMessage(&(currentProcess->msg_inbox),senderAddr);
    }

    if(msg==NULL) {}  //DEBUG: aggiungere paragrafo 5.5

    if(whereToSave != (memaddr*)NULL) whereToSave = msg->m_payload;
    freeMsg(msg);

    return msg->m_sender;
}

/*
Gestisce le eccezioni, capisce che tipo di eccezione è, quindi passa la palla ad un'altra funzione, in base al numero dell'excCode
Section 4
*/
void exceptionHandler(){
    unsigned int excCode = getCAUSE();

    switch(excCode){
        //Interrupts
        case IOINTERRUPTS:                                                  //DEBUG: fatto (credo)
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
            if((currentProcess->p_s.status & USERPON) != 0){
		        setCAUSE(PRIVINSTR);
                exceptionHandler();
	        }
            //SYSCALL fatta in kernel-mode
            int a0 = currentProcess->p_s.reg_a0;
            int v0 = currentProcess->p_s.reg_v0;
            if(a0 == SENDMESSAGE)           //a0 = -1
		        v0 = SYS1_sendMessage();
	        else if(a0 == RECEIVEMESSAGE)   //a0 = -2
		        v0 = SYS2_receiveMessage();
            else 
                kill(GENERALEXCEPT);        //a0 != -1, -2  (Sezione 9.1)
            break;
    }
}