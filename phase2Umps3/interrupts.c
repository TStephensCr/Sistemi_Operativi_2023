#include "const.h"
//sta roba dell'and non ha senso...
int interrupt_line(/*interrupt*/){
    if(getCAUSE() /*&& interrupt*/){
        
    }
}

void interrupt_eh(){
    //controlli tutte le lines?? il prof dice che determini le lines con il bitwise and ma quello lo fai su tutte le lines bruh
    //una volta determinate le lines che hanno un interrupt prendi quello con priorità maggiore 
    //NB terminal transmission >> terminal receipt
}
