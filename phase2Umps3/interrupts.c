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
    if(getCAUSE() && LOCALTIMERINT){//line 1    plt interrupt
        currentProcess->p_sib = EXCEPTION_STATE;
        currentProcess->p_time += tempopassato();
        insertProcQ(&readyQueue, currentProcess);
        currentProcess = NULL;
        address = get_numdevice(1);
    }                                               //lascia in questo ordine per la priorità
    else if(getCAUSE() && TIMERINTERRUPT)//line 2   interval timer interrupt
        address = get_numdevice(2);
    else if(getCAUSE() && DISKINTERRUPT)//line 3
        NT_handler(3);
        address = get_numdevice(3);
    else if(getCAUSE() && FLASHINTERRUPT)//line 4 linea 5 skippabile perché il nostro os non avrà interazione con intrnet
        NT_handler(4);
        address = get_numdevice(4);
    else if(getCAUSE() && PRINTINTERRUPT)// line 6
        NT_handler(6);
        address = get_numdevice(6);
    else if(getCAUSE() && TERMINTERRUPT)// line 7
        NT_handler(7);
        address = get_numdevice(7);
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


void NT_handler(int ip){
    int line, numero, mask;
    int devAddrBase = get_numdevice(line);
    status = get_status(devAddrBase);
    set_status(ACK);
    /**
     * manda messaggio e sblocca il pcb in attes di questo device
     * aggiorna il registro v0 del pcb con status
     * metti il pcb sbloccato nella ready queue cambiando lo state da blocked a ready
     * ridai il controllo al current process facendo un LDST sull'exception state salvato (si trova all'inizio della BIOSDATAPAGE)
    */
}


int get_numdevice(int line){
    switch (line){
        case DEV1ON:
            return int devAddrBase = 0x10000054 + ((1 - 3) * 0x80) + (devicenumber * 0x10);        
            break;
        case DEV2ON:
            return devAddrBase = 0x10000054 + ((2 - 3) * 0x80) + (devicenumber * 0x10);
            break;
        case DEV3ON:
            return devAddrBase = 0x10000054 + ((3 - 3) * 0x80) + (devicenumber * 0x10);       
            break;
        case DEV4ON:
            return devAddrBase = 0x10000054 + ((4 - 3) * 0x80) + (devicenumber* 0x10);       
            break;
        case DEV5ON:
            return devAddrBase = 0x10000054 + ((5 - 3) * 0x80) + (devicenumber * 0x10);       
            break;
        case DEV6ON:
            return devAddrBase = 0x10000054 + ((6 - 3) * 0x80) + (devicenumber * 0x10);       
            break;
        case DEV7ON:
            return devAddrBase = 0x10000054 + ((7 - 3) * 0x80) + (devicenumber * 0x10);       
            break;
        
    default:
        break;
    }
    
}
