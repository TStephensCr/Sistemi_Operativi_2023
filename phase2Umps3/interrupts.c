#include "./headers/interrupts.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
unsigned int *intLaneMapped = (memaddr*)(INTDEVBITMAP + (0x4 * (line - 3)))
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
    for(int line = 1; line < 8 ; line++){
        startinterrupt();
        if(getCAUSE() & LOCALTIMERINT){//line 1    plt interrupt FINITI
            currentProcess->p_sib = (state_t *)BIOSDATAPAGE;//exception state
            currentProcess->p_time = TIMESLICE;//se da problemi prova ad assegnare il valore ad una variabile e poi assegnare la variabile

            insertProcQ(&readyQueue, currentProcess);
            currentProcess = NULL;
            scheduler();
        }                                               //lascia in questo ordine per la priorità
        else if(getCAUSE() & TIMERINTERRUPT){//line 2   interval timer interrupt
            //100 millisecondi nell'interval timer
            LDIT(PSECOND);
            pcb_t *unblocked_pcb;
            while ((unblocked_pcb = removeProcQ(&PseudoClockWP)) != NULL) {
            //sblocco di tutti i processi in attesa dello pseudoclock
                send(ssi_pcb, unblocked_pcb, 0);
                insertProcQ(&readyQueue, unblocked_pcb);
                softBlockCount--;
            }

            //ritorna il controllo al current process
            LDST((state_t *)BIOSDATAPAGE);
            }
        else if(getCAUSE() & DISKINTERRUPT){//line 3
            NT_handler(line);
            int address = get_numdevice(3);
        }
        else if(getCAUSE() & FLASHINTERRUPT){//line 4 linea 5 skippabile perché il nostro os non avrà interazione con intrnet
            NT_handler(line);
            int address = get_numdevice(4);
        }
        else if(getCAUSE() & PRINTINTERRUPT){// line 6
            NT_handler(line);
            int address = get_numdevice(6);
        }
        else if(getCAUSE() & TERMINTERRUPT){// line 7 per i terminal devices devi fare una roba diversa
            NT_handler(line);
            int address = get_numdevice(7);
        }
        endinterrupt();
    }
}


//aggiorna il tempo di esecuzione del current process così da non dargli il tempo di esecuzione dell'interrupt
void startinterrupt(){
  //se il processo corrente non è nullo allora aggiorna il tempo
  //poi cambia lo status  
  if(currentProcess != NULL){
    currentProcess->p_time += tempopassato();
  }
  setSTATUS (getSTATUS() & ~TEBITON);
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
    int num = get_numdevice(line);
    unsigned int devAddrBase = 0x10000054 + ((line - 3) * 0x80) + (num * 0x10);
    //2
    
    //3
    
    //4
    pcb_t* waitingProcess = blockedpcbs[(line-3) * 4 + num];

    if(waitingProcess != NULL){
        waitingProcess->p_s.v0 = status;//status è da rivedere 
        insertProcQ(&readyQueue,waitingProcess);
    }
    if(currentProcess==NULL)//finito
        scheduler();
    else                    //non finito
        LDST((state_t*) BIOSDATAPAGE);

    /**
     * manda messaggio e sblocca il pcb in attes di questo device
     * aggiorna il registro v0 del pcb con status
     * metti il pcb sbloccato nella ready queue cambiando lo state da blocked a ready
     * ridai il controllo al current process facendo un LDST sull'exception state salvato (si trova all'inizio della BIOSDATAPAGE)
    */
}




int get_numdevice(int line){
     devregarea_t *dra = (devregarea_t *)BUS_REG_RAM_BASE;
    //accedo alla bitmap dei device per la linea su cui è stato rilevato un interrupt
    unsigned int intdevbitmap = dra->interrupt_dev[line - 3];
    unsigned int dstatus;
    unsigned int dnumber;
    for(int i = 0; i<8;i++){
        if(intdevbitmap & intconst[i]){
            dnumber = i;
        
        }
    }
    
}
