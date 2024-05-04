#include "./headers/exceptions.h"

//DEBUG: mancano 5.4 e 5.5 ma lo faccio direttamente quando facciamo debug che devo capire delle cose prima

void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t*) 0x0FFFF000);
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
        current_process->p_supportStruct->sup_exceptState[index] = *(state_t*)BIOSDATAPAGE;
        //Sezione 7.3.4 del manuale: (u-int stackPtr,u-int status,u-int pc)
        LDCXT(current_process->p_supportStruct->sup_exceptContext[index].stackPtr,
            current_process->p_supportStruct->sup_exceptContext[index].status,
            current_process->p_supportStruct->sup_exceptContext[index].pc);
    }
}

static int SYS1_sendMessage(){
    pcb_PTR destinationAddr = (pcb_PTR)current_process->p_s.reg_a1;
    unsigned int payload = (unsigned int)current_process->p_s.reg_a2;

    if(pcbIsInList(destinationAddr,&pcbFree_h)==1) return DEST_NOT_EXIST;

    msg_t *msg = allocMsg();
    if(msg==NULL) return MSGNOGOOD;  //non ho più messaggi disponibili

    //il pcb destinatario va svegliato mettendolo nella readyQueue
    if(pcbIsInList(destinationAddr,&readyQueue)!=1) insertProcQ(&readyQueue, destinationAddr);
    
    msg->m_sender = current_process;
    msg->m_payload = payload;
    insertMessage(&(destinationAddr->msg_inbox), msg);
    return 0;
} 


static int SYS2_receiveMessage(){
    pcb_PTR senderAddr = (pcb_PTR)current_process->p_s.reg_a1;
    memaddr* whereToSave = (memaddr*)current_process->p_s.reg_a2;

    msg_PTR msg;

    if(senderAddr == (pcb_PTR)ANYMESSAGE){ //prendiamo il primo messaggio nella lista
        msg = popMessage(&(current_process->msg_inbox),NULL);
    }else{
        msg = popMessage(&(current_process->msg_inbox),senderAddr);
    }

    if(msg==NULL) {}  //DEBUG: aggiungere paragrafo 5.5

    if(msg->m_payload != (memaddr)NULL) *whereToSave = msg->m_payload;
    freeMsg(msg);

    return (memaddr)msg->m_sender;
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
            if((current_process->p_s.status & USERPON) != 0){
		        setCAUSE(PRIVINSTR);
                exceptionHandler();
	        }
            //SYSCALL fatta in kernel-mode
            int a0 = current_process->p_s.reg_a0;
            if(a0 == SENDMESSAGE)         //a0 = -1
                current_process->p_s.reg_v0 = SYS1_sendMessage();
            else if(a0 == RECEIVEMESSAGE)  //a0 = -2
                current_process->p_s.reg_v0 = SYS2_receiveMessage();        //v0
            else kill(GENERALEXCEPT);        //a0 != -1, -2  (Sezione 9.1)
            break;
    }
}