#include "./headers/interrupts.h"

extern int process_count;
extern unsigned int softBlockCount;
extern struct list_head readyQueue;
extern pcb_PTR current_process;
extern pcb_t ssi_pcb;
//interrupting device bitmap è una matrice di booleani in cui se c'è un 1 allora c'è un interrupt pending


/**
 * questa funzione qui sotto non solo mi ritorna il device con un pending interrupt in quella line ma mi ritorna quello con più priorità quindi poi lo passo al NTHANDLER già gestito
    inoltre posso lasciare la bitmap qui dentro e chillarmela
*/
static int get_numdevice(int line){

    int intconst[8] = { 0x00000001,//device 0
                    0x00000002,//device 1
                    0x00000004,//device 2
                    0x00000008,//device 3
                    0x00000010,//device 4
                    0x00000020,//device 5
                    0x00000040,//device 6
                    0x00000080};//device 7

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

//salvataggio dello stato
//prendiamo i parametri dalla libreria types.h di umps3
void copyState(state_t* end, state_t* start) {
    //copia di tutti i parametri di uno stato nell'altro
    end->entry_hi = start->entry_hi;
    end->cause = start->cause;
    end->status = start->status;
    end->pc_epc = start->pc_epc;
    end->hi = start->hi;
    end->lo = start->lo;
    for(int i = 0; i < STATE_GPR_LEN; i++) 
        end->gpr[i] = start->gpr[i];
}

//tempo che serve a svolgere  il processo
static cpu_t tempopassato(){
    int attuale;
    STCK(attuale);
    int risultante = attuale - ultimo;
    STCK (ultimo);
    return risultante;
}

static void removeBlocked(pcb_t *pcb, pcb_PTR blockedpcbs[SEMDEVLEN][2]) {
    if (pcb != NULL) {
        // cerca in blockedpcbs il PCB
        for (int i = 0; i < SEMDEVLEN - 1; i++) {
            for (int j = 0; j < 2; j++) {
                if (blockedpcbs[i][j] == pcb) {
                    // Una volta trovato, al suo slot assegnamo NULL
                    blockedpcbs[i][j] = NULL;
                    // Assumendo che ci sia una sola volta,
                    // si può ritornare dopo aver rimosso
                    return;
                }
            }
        }
    }
}

static void sbloccapcb(int deviceNum, int interruptLine, pcb_PTR blockedpcbs[SEMDEVLEN-1][2]) {
    // calcolo l'indice dell'array blockedpcbs
    int devIndex = EXT_IL_INDEX(interruptLine) * N_DEV_PER_IL + deviceNum;
    // controlli bounds
    if (devIndex >= 0 && devIndex < SEMDEVLEN) {
        // salviamo il puntatore
        pcb_t *pcb1 = blockedpcbs[devIndex][0];
        pcb_t *pcb2 = blockedpcbs[devIndex][1];
        // rimuoviamo il pcb dai pcb bloccati
        if (pcb1 != NULL) {
            removeBlocked(pcb1, blockedpcbs);
        }
        if (pcb2 != NULL) {
            removeBlocked(pcb2, blockedpcbs);
        }
        // e lo mettiamo ready
        if (pcb1 != NULL) {
            insertProcQ(&readyQueue, pcb1);
        }
        if (pcb2 != NULL) {
            insertProcQ(&readyQueue, pcb2);
        }
    }
}


//aggiorna il tempo di esecuzione del current process così da non dargli il tempo di esecuzione dell'interrupt
static void startinterrupt(){
  //se il processo corrente non è nullo allora aggiorna il tempo
  //poi cambia lo status  
  if(current_process != NULL){
    current_process->p_time += tempopassato();
  }
  setSTATUS (getSTATUS() & ~TEBITON);
}

//fine interrupt e riprende il processo interrotto, se nullo chiama lo scheduler
static void endinterrupt(){
    setSTATUS (getSTATUS() || TEBITON);
    STCK (ultimo);
    if (current_process != NULL)
        LDST ((state_t *)BIOSDATAPAGE);
    else
        scheduler ();
}


static void NT_handler(int line){
    //1
    int num = get_numdevice(line);
    dtpreg_t *device_register = (dtpreg_t *)DEV_REG_ADDR(line, num);
    unsigned int dstatus = device_register->status;
    pcb_t* waitingProcess = blockedpcbs[(line-3) * 4 + num][0];//non sono sicuro su questo 0

    if(line==7){ // device terminali
        //gestione interrupt di tutti gli altri dispositivi I/O
        //gestione interrupt terminale --> 2 sub-devices
        device_register->command = ACK;
        sbloccapcb(num,line, blockedpcbs);
    }
   

    if(waitingProcess != NULL){
        waitingProcess->p_s.reg_v0 = dstatus; 
        sendMsg(ssi_pcb, waitingProcess, (memaddr)(dstatus));
        insertProcQ(&readyQueue,waitingProcess);
        softBlockCount--;
    }
    
    if(current_process==NULL)
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
            current_process->p_time += tempopassato();
            state_t *exceptionstate = (state_t* )BIOSDATAPAGE;
            copyState(&(current_process->p_s), exceptionstate);
            insertProcQ(&readyQueue, current_process);
            scheduler();
        }                                               //lascia in questo ordine per la priorità
        else if(getCAUSE() & TIMERINTERRUPT){//line 2   interval timer interrupt
            //100 millisecondi nell'interval timer
            LDIT(PSECOND);
            pcb_t *unblocked_pcb;
            while ((unblocked_pcb = removeProcQ(&PseudoClockWP)) != NULL) {
            //sblocco di tutti i processi in attesa dello pseudoclock
                sendMsg(ssi_pcb, unblocked_pcb, 0);
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