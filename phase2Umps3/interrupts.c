#include <umps3/umps/cp0.h>
#include "initial.c"

void interrupthandler();//funzioni abbastanza ovvie comunque sotto spiego cosa fanno
void PLT_handler();
void IT_handler();
void NT_handler();//forse qui va qualcosa in input
void startinterrupt();
void endinterrupt();
void ritorna_device(int *);

//DETERMINA IL TIPO DI INTERRUPT E ASSEGNA 
void interrupthandler(){
    startinterrupt();
    if(getCAUSE() && LOCALTIMERINT) //lascia in questo ordine per la priorità
        PLT_handler();
    else if(getCAUSE() && TIMERINTERRUPT)
        IT_handler();
    else
        NT_handler();
    endinterrupt();
}


//aggiorna il tempo di esecuzione del current process così da non dargli il tempo di esecuzione dell'interrupt
void startinterrupt(){
  //se il processo corrente non è nullo allora aggiorna il tempo
  //poi cambia lo status  
  if(currentProcess != NULL){
    currentProcess->p_time += tempopassato();
  }
  setSTATUS (getSTATUS() && ~TEBITON);
}

//fine interrupt e riprende il processo interrotto, se nullo chiama lo scheduler
void endinterrupt(){
    setSTATUS (getSTATUS() || TEBITON);
    STCK (ultimo);
    if (currentProcess != NULL)
        LDST (EXCEPTION_STATE);
    else
        scheduler ();
}

void PLT_handler(){
    currentProcess->p_sib = EXCEPTION_STATE;
    currentProcess->p_time += tempopassato();

    insertProcQ(&readyQueue, currentProcess);
    currentProcess = NULL;
    
}

void IT_handler(){

}