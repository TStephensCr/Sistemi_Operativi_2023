#include <const.h>

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


//setta l'interruzione
void startinterrupt(){
  //se il processo corrente non è nullo allora aggiorna il tempo
  //poi cambia lo status  
}

void PLT_handler(){

}

void IT_handler(){

}