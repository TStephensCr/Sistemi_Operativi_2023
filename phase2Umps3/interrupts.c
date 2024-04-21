#include "./headers/interrupts.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
extern cpu_t ultimo;

//interrupting device bitmap è una matrice di booleani in cui se c'è un 1 allora c'è un interrupt pending

//tempo che serve a svolgere  il processo
cpu_t tempopassato(){
    cpu_t attuale;
    STCK(attuale);
    cpu_t risultante = attuale - ultimo;
    STCK (ultimo);
    return risultante;
}

//DETERMINA CHE TIPO DI INTERRUPT è PENDING IN BASE ALLA LINEA
//POI CONTROLLA GLI 8 DEVICE PER CAPIRE A QUALE APPARTIENE
// QUALCOSA NOPN VA T.T     
// STRUTTURE DEI TERMINAL DEVICES A PAG 58 line 7
// TUTTI GLI EXTERNAL DEVICE HANNO LA STESSSA STRUTTURA 
// external devices line 3-6
// in ogni linea controlla quale device ha un interrupt pending attraverso la interrupting device bit map
// 
void interrupthandler(){
    startinterrupt();
    if(getCAUSE() && LOCALTIMERINT){//line 1    plt interrupt FINITI
        currentProcess->p_sib = (state_t *)BIOSDATAPAGE;//exception state
        currentProcess->p_time = TIMESLICE;
        insertProcQ(&readyQueue, currentProcess);
        currentProcess = NULL;
        scheduler();
    }                                               //lascia in questo ordine per la priorità
    else if(getCAUSE() && TIMERINTERRUPT){//line 2   interval timer interrupt
        //100 millisecondi nell'interval timer
        LDIT(PSECOND);

        //sblocca i pcb in attesa di uno pseudo clock tick


        //ritorna il controllo al current process
        LDST((state_t *)BIOSDATAPAGE);

        }
    else if(getCAUSE() && DISKINTERRUPT){//line 3
        NT_handler(3);
        int address = get_numdevice(3);
    }
    else if(getCAUSE() && FLASHINTERRUPT){//line 4 linea 5 skippabile perché il nostro os non avrà interazione con intrnet
        NT_handler(4);
        int address = get_numdevice(4);
    }
    else if(getCAUSE() && PRINTINTERRUPT){// line 6
        NT_handler(6);
        int address = get_numdevice(6);
    }
    else if(getCAUSE() && TERMINTERRUPT){// line 7 per i terminal devices devi fare una roba diversa
        NT_handler(7);
        int address = get_numdevice(7);
    }
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
        LDST ((state_t *)BIOSDATAPAGE);
    else
        scheduler ();
}

int MAXPNT(){ // ritorna la linea in cui si trova il NTI pending con priorità più alta
    for (int line = 3; line < 8; line++){
        if (getCAUSE() & (1 << line) & line != 5){ //i device alla line 5 sono gli ethernet devices e il nostro OS non avrà interazioni con internet quindi ez
            return line;
        }
    }
    return -1;
}



void NT_handler(int line){
    //1
    devAddrBase = 0x10000054 + ((line - 3) * 0x80) + (get_numdevice() * 0x10);
    //2
    //3
    //4

    /**
     * manda messaggio e sblocca il pcb in attes di questo device
     * aggiorna il registro v0 del pcb con status
     * metti il pcb sbloccato nella ready queue cambiando lo state da blocked a ready
     * ridai il controllo al current process facendo un LDST sull'exception state salvato (si trova all'inizio della BIOSDATAPAGE)
    */
}

void passupordie(){
    if(currentProcess->p_supportStruct == NULL){ // parte "die" del codice

    }else{ //parte "pass up"

    }
}


int get_numdevice(int line){
    devregarea_t *area_registrobus = (devregarea_t *)BUS_REG_RAM_BASE;
    unsigned int bitmap = bus_reg_area->interrupt_dev[EXT_IL_INDEX (line)];
    for (int number = 0, mask = 1; number < N_DEV_PER_IL; number++, mask <<= 1)
        if (bitmap & mask)
            return number;
    return -1;
    
}
