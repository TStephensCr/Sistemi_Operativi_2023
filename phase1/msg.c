#include "./headers/msg.h"

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);

void initMsgs() {
    //Inizializziamo la lista msgFree_h con i puntatori del suo unico elemento che puntano a se stesso
    INIT_LIST_HEAD(&msgFree_h);
    //Inseriamo gli elementi dell'array statico nella lista
    for(int i = 0; i < MAXMESSAGES; i++) list_add_tail(&(msgTable[i].m_list), &msgFree_h);
}

void freeMsg(msg_t *m) {
    //Inseriamo in coda a msgFree_h l'elemento m
    list_add_tail(&(m->m_list), &msgFree_h);
}

msg_t *allocMsg() {
    if(list_empty(&msgFree_h)) return NULL;     //Se msgFree è vuota, ritotno NULL
    //Altrimenti rimuovo un elemento dalla lista dei messaggi e lo inizializzo
    msg_t *msg = container_of(msgFree_h.next, msg_t, m_list);
    list_del(&(msg->m_list));

    INIT_LIST_HEAD(&(msg->m_list));
    msg->m_sender=NULL;
    msg->m_payload=0;

    return msg;
}

void mkEmptyMessageQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}

int emptyMessageQ(struct list_head *head) {
    return list_empty(head);
}

void insertMessage(struct list_head *head, msg_t *m) {
  list_add_tail(&(m->m_list),head);
}

void pushMessage(struct list_head *head, msg_t *m) {
  list_add(&(m->m_list),head);
}

//Controlliamo che la lista non sia vuota, poi torniamo il puntatore al primo messaggio in lista
msg_t *headMessage(struct list_head *head) {
    if(emptyMessageQ(head)==1) return NULL;
    return container_of(list_next(head), msg_t, m_list);
}

msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {
    //Controlliamo se la lista è vuota, in questo caso torniamo NULL
    if(emptyMessageQ(head)==0){
        //Altrimenti controlliamo che p_ptr sia null, in questo caso ritorniamo il primo messaggio in lista
        msg_t *res=NULL;
        if(p_ptr==NULL){
            res=headMessage(head);
            list_del(&res->m_list); 
            return res; 
        }
        else{
            //Altrimenti cerchiamo il p_ptr tra i m_sender e quando lo troviamo, lo restituiamo
            list_for_each_entry(res, head, m_list){
                if(res->m_sender == p_ptr){
                    list_del(&res->m_list);
                    return res;
                }
            }
        }
    }
    return NULL;
}