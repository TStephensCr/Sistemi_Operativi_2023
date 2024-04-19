#include "./headers/exceptions.h"

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
        case SYSEXCEPTION:                                                  //DEBUG: manca questo
            break;
    }
}