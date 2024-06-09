#include "./headers/initial.h"

int process_count;                  //Process Count, numero di processi attivi e non terminati
unsigned int softBlockCount;                //Soft-Block Count, numero di processi in waiting per I/O o per tempo esaurito
LIST_HEAD(readyQueue);                //Ready Queue, puntatore alla coda di porcessi in ready
pcb_PTR current_process;                     //Current Process, punta al processo in running
pcb_PTR blockedpcbs[SEMDEVLEN-1];
/*
0-7     DISKS
8-15    FLASH
16-23   NETWORK
24-31   PRINTERS
32-48   TERMINALS
            32  Scrittura primo terminale
            33  Lettura primo terminale
            34  Scrittura secondo terminale
            35  Lettura secondo terminale
            ...
            46  Scrittura ottavo terminale
            47  Lettura ottavo terminale
*/
LIST_HEAD(waitingForClock);
int ultimo;
struct list_head PseudoClockWP;
struct list_head p_list;              //ultimo TOD
pcb_PTR ssi_pcb;
int start;

static void second_pcb(){
    pcb_PTR new_pcb = allocPcb();
    
    RAMTOP(new_pcb->p_s.reg_sp);    

    new_pcb->p_s.reg_sp -= (2 * PAGESIZE);   //DEBUG
    
    //Spiegati a sezione 2.3 del manuale, la | fa la or Bit-a-Bit delle costanti che inserisco, in modo da attivare i bit giusti
    new_pcb->p_s.status = ALLOFF | IECON | IMON | TEBITON;   //Interrupt(bit e InterruptMask), KernelMode e LocalTimer abilitati
    
    new_pcb->p_s.pc_epc = (memaddr)test;    
    //new_pcb->p_s.reg_t9 = (memaddr)test;
    ssi_pcb-> p_s.gpr[24] = new_pcb-> p_s.pc_epc;
    
    new_pcb->p_parent=NULL;
    new_pcb->p_pid = 2;
    
    new_pcb->p_supportStruct=NULL;

    insertProcQ(&readyQueue, new_pcb);
    process_count++;
}

static void first_pcb(){
    ssi_pcb = allocPcb();
    RAMTOP(ssi_pcb->p_s.reg_sp);

    //Spiegati a sezione 2.3 del manuale, la | fa la or Bit-a-Bit delle costanti che inserisco, in modo da attivare i bit giusti
    ssi_pcb->p_s.status = ALLOFF | IEPON | IMON | TEBITON;

    ssi_pcb->p_s.pc_epc = (memaddr)remoteProcedureCall;    //its PC set to the address of SSI_function_entry_point

    ssi_pcb-> p_s.gpr[24] = ssi_pcb-> p_s.pc_epc;
  
    ssi_pcb->p_parent=NULL;
    ssi_pcb->p_pid = 1;
    ssi_pcb->p_supportStruct=NULL;

    insertProcQ(&readyQueue, ssi_pcb); 
    process_count++;
}

int main(void){
    passupvector_t *passupvect = (passupvector_t *)PASSUPVECTOR;
    passupvect->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvect->exception_stackPtr = (memaddr)KERNELSTACK;
    passupvect->exception_handler = (memaddr)exceptionHandler;

    passupvect->tlb_refill_stackPtr = (memaddr)KERNELSTACK;

    initPcbs();
    initMsgs();

    process_count=0;
    softBlockCount=0;
    mkEmptyProcQ(&readyQueue);
    current_process=NULL;
    for(int i=0;i<SEMDEVLEN-1;i++){
        blockedpcbs[i]=NULL;
    }

    mkEmptyProcQ(&readyQueue);
    mkEmptyProcQ(&waitingForClock);
    LDIT(PSECOND);

    first_pcb();        //ssi_pcb
    second_pcb();

    scheduler();
    return 0;
}