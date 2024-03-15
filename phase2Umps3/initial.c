#include "./headers/initial.h"
#include "const.h"
/*
MANCA: (DEBUG)
2.1
6.1
7 (RAMTOP)
7.1
*/


extern void test ();

void second_pcb(){
    pcb_PTR new_pcb = allocPcb();
    
    RAMTOP(new_pcb->p_s.reg_sp);    //Da togliere e mettere quella sotto ma corretta
    //new_pcb->p_s.reg_sp = RAMTOP(new_pcb->p_s.reg_sp) - (2 * FRAMESIZE); DEBUG: non so come sostituire FRAMESIZE, dovrebbe essere la grandezza del primo pcb inserito
    
    //Spiegati a sezione 2.3 del manuale, la | fa la or Bit-a-Bit delle costanti che inserisco, in modo da attivare i bit giusti
    new_pcb->p_s.status = ALLOFF | IECON | IMON | TEBITON;   //Interrupt(bit e InterruptMask), KernelMode e LocalTimer abilitati
    //new_pcb->p_s.status = ALLOFF | IEPON | IMON | TEBITON; //DEBUG: Secondo me ha senso quella, ma manuale fa intendere questa verisone, da capire
    
    new_pcb->p_s.pc_epc = (memaddr)test;    
    new_pcb->p_s.reg_t9 = (memaddr)test;
    
    new_pcb->p_parent=NULL;
    //new_pcb->p_child=NULL;      //DEBUG: Non capisco perché mi dia errore
    //new_pcb->p_sib=NULL;        //
    new_pcb->p_time=0;
    new_pcb->p_supportStruct=NULL;

    insertProcQ(&readyQueue, new_pcb);
    processCount++;
}

void first_pcb(){
    pcb_PTR new_pcb = allocPcb();

    RAMTOP(new_pcb->p_s.reg_sp);
    
    //Spiegati a sezione 2.3 del manuale, la | fa la or Bit-a-Bit delle costanti che inserisco, in modo da attivare i bit giusti
    new_pcb->p_s.status = ALLOFF | IECON | IMON;   //Interrupt(bit e InterruptMask) e KernelMode abilitati
    //new_pcb->p_s.status = ALLOFF | IEPON | IMON; //DEBUG: Secondo me ha senso quella, ma manuale fa intendere questa verisone, da capire
    
    new_pcb->p_s.pc_epc = (memaddr)test;    //its PC set to the address of SSI_function_entry_point
    new_pcb->p_s.reg_t9 = (memaddr)test;
    
    new_pcb->p_parent=NULL;
    //new_pcb->p_child=NULL;      //DEBUG: Non capisco perché mi dia errore
    //new_pcb->p_sib=NULL;        //
    new_pcb->p_time=0;
    new_pcb->p_supportStruct=NULL;

    insertProcQ(&readyQueue, new_pcb);
    processCount++;
}

//tempo che serve a svolgere  il processo
cpu_t tempopassato(){
    cpu_t attuale;
    STCK(attuale);
    cpu_t risultante = attuale - ultimo;
    STCK (ultimo);
    return risultante;
}

int main(void){
    passupvector_t *passupvect = (passupvector_t *)PASSUPVECTOR;
    passupvect->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvect->exception_stackPtr = (memaddr)KERNELSTACK;
    //passupvect->exception_handler = (memaddr)exceptionHandler;    //DEBUG: togliere commento quando exceptionHandler pronto

    passupvect->tlb_refill_stackPtr = (memaddr)KERNELSTACK;

    initPcbs();
    initMsgs();

    processCount=0;
    softBlockCount=0;
    mkEmptyProcQ(&readyQueue);
    currentProcess=NULL;
    for(int i=0;i<SEMDEVLEN;i++){
        blockedpcbs[i][0]=NULL;
        blockedpcbs[i][1]=NULL;
    }
    
    LDIT(PSECOND);

    first_pcb();
    second_pcb();
    
    scheduler();

    return 0;
}