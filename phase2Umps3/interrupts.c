#include "./headers/interrupts.h"

extern unsigned int processCount;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR currentProcess;
extern pcb_PTR blockedpcbs[SEMDEVLEN][2];
extern pcb_t ssi_pcb;
//interrupting device bitmap è una matrice di booleani in cui se c'è un 1 allora c'è un interrupt pending

//tempo che serve a svolgere  il processo
cpu_t tempopassato(){
    int attuale;
    STCK(attuale);
    int risultante = attuale - ultimo;
    STCK (ultimo);
    return risultante;
}

void sbloccapcb(int deviceNum, int interruptLine, pcb_PTR blockedpcbs[SEMDEVLEN][2]) {
    // Calculate the index of the device in the blockedpcbs array
    int devIndex = EXT_IL_INDEX(interruptLine) * N_DEV_PER_IL + deviceNum;

    // Check bounds to avoid potential out-of-bounds access
    if (devIndex >= 0 && devIndex < SEMDEVLEN) {
        // Get the PCB pointers from the blockedpcbs array
        pcb_t *pcb1 = blockedpcbs[devIndex][0];
        pcb_t *pcb2 = blockedpcbs[devIndex][1];

        // Remove the PCBs from the blocked queue
        if (pcb1 != NULL) {
            removeBlocked(pcb1);
        }
        if (pcb2 != NULL) {
            removeBlocked(pcb2);
        }

        // Insert the PCBs into the ready queue
        if (pcb1 != NULL) {
            insertProcQ(&readyQueue, pcb1);
        }
        if (pcb2 != NULL) {
            insertProcQ(&readyQueue, pcb2);
        }
    }
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
            setTIMER(-1); //ACK interrupt
            currentProcess->p_time += tempopassato();
            state_t *exceptionstate = (state_t* )BIOSDATAPAGE;
            saveState(&(currentProcess->p_s), exceptionstate);
            insertProcQ(&readyQueue, currentProcess);
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
        }
        else if(getCAUSE() & FLASHINTERRUPT){//line 4 linea 5 skippabile perché il nostro os non avrà interazione con intrnet
            NT_handler(line);
        }
        else if(getCAUSE() & PRINTINTERRUPT){// line 6
            NT_handler(line);
        }
        else if(getCAUSE() & TERMINTERRUPT){// line 7 per i terminal devices devi fare una roba diversa
            NT_handler(line);
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


void NT_handler(int line){
    //1
    int num = get_numdevice(line);
    unsigned int dstatus;
    pcb_t* waitingProcess = blockedpcbs[(line-3) * 4 + num];

    if (line!=7){ //per i device non terminali
        //accedo al device register
        dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(line, num);
        unsigned int dstatus = device_register->status;          //salvo lo status
        device_register->command = ACK;//acknowledged
    }else{ // device terminali
        dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(line, num);
        //gestione interrupt di tutti gli altri dispositivi I/O
        dstatus = device_register->status;
        device_register->command = ACK;
        //gestione interrupt terminale --> 2 sub-devices
        if(((device_register->transm_status) & 0x000000FF) == 5){ //ultimi 8 bit contengono il codice dello status
            //output terminale
            dstatus = device_register->transm_status;
            device_register->transm_command = ACK;
            waitingProcess = sbloccapcb(num,line, blockedpcbs);
        }else{
            //input terminale
            dstatus = device_register->recv_status;
            device_register->recv_command = ACK;
            waitingProcess = sbloccapcb(num,line, blockedpcbs);
        }
    }

    if(waitingProcess != NULL){
        waitingProcess->p_s. = dstatus; 
        send(ssi_pcb, waitingProcess, (memaddr)(dstatus));
        insertProcQ(&readyQueue,waitingProcess);
        softBlockCount--;
    }
    
    if(currentProcess==NULL)
        scheduler();
    else                    
        LDST((state_t*) BIOSDATAPAGE);
    /**
     * manda messaggio e sblocca il pcb in attes di questo device
     * aggiorna il registro v0 del pcb con status
     * metti il pcb sbloccato nella ready queue cambiando lo state da blocked a ready
     * ridai il controllo al current process facendo un LDST sull'exception state salvato (si trova all'inizio della BIOSDATAPAGE)
    */
}


//questa funzione ui sotto non solo mi ritorna il device con un pending interrupt in quella line ma mi ritorna 
//quello con più priorità quindi poi lo passo al NTHANDLER già gestito
//inoltre posso lasciare la bitmap qui dentro e chillarmela

int get_numdevice(int line){
     devregarea_t *dra = (devregarea_t *)BUS_REG_RAM_BASE;
    //accedo alla bitmap dei device per la linea su cui è stato rilevato un interrupt
    unsigned int intdevbitmap = dra->interrupt_dev[line - 3];
    unsigned int dnumber;
    for(int i = 0; i<=8;i++){
        if(intdevbitmap & intconst[i]){
            dnumber = i;
        }
    }
    return dnumber;
}
