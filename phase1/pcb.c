#include "./headers/pcb.h"

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

void initPcbs() {
    INIT_LIST_HEAD(&pcbFree_h);
    for(int i = 0; i < MAXPROC; i++){
        list_add_tail(&pcbTable[i].p_list, &pcbFree_h);
        pcbTable[i].p_pid = i + 1;
    }
}

void freePcb(pcb_t *p) {
    list_add_tail(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb() {
    if(list_empty(&pcbFree_h)){//FIX TEMPORANEA
        return NULL;
    } else {
        pcb_t *p = container_of(pcbFree_h.next, pcb_t, p_list);
        list_del(&p->p_list);
        INIT_LIST_HEAD(&p->msg_inbox);
        p->p_parent = NULL;
        INIT_LIST_HEAD(&p->p_child);
        INIT_LIST_HEAD(&p->p_sib);
        p->p_supportStruct = NULL;
        p->p_time = 0;
        p->p_pid = p - pcbTable + 1;
        return p;
    }
}

void mkEmptyProcQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *head) {
    return list_empty(head);
}

void insertProcQ(struct list_head *head, pcb_t *p) {
    list_add_tail(&p->p_list, head);
}

pcb_t *headProcQ(struct list_head *head) {
    if(list_empty(head)){//prova emptyProcQ semmai
        return NULL;
    } else {
        return container_of(list_next(head), pcb_t, p_list);
    }
}

pcb_t *removeProcQ(struct list_head *head) {
    if(list_empty(head)){
        return NULL;
    } else {
        pcb_t *p = headProcQ(head);
        list_del(&p->p_list);
        return p;
    }
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
    if(emptyProcQ(head))
        return NULL;
    if(headProcQ(head) == p){
        list_del(&p->p_list);
        return p;
    }
    pcb_t *tmp;
    list_for_each_entry(tmp, head, p_list){
        if(tmp == p){
            list_del(&p->p_list);
            return p;
        }
    }
    return NULL;
}

int emptyChild(pcb_t *p) {
    return list_empty(&p->p_child);
}

void insertChild(pcb_t *prnt, pcb_t *p) {
    list_add(&p->p_list, &prnt->p_child);
    p->p_parent = prnt;
    //fin qui va bene ma c'è un bordello per p_sib, è quello su cui da errore
}

pcb_t *removeChild(pcb_t *p) {
}

pcb_t *outChild(pcb_t *p) {
}
