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
    p->p_parent = prnt;
    list_add_tail(&p->p_sib, &prnt->p_child);
}

pcb_t *removeChild(pcb_t *p) {
    if(emptyProcQ(&p->p_child))
        return NULL;
    else{
        //salvo figlio prima di rimuoverlo per poi returnarlo
        pcb_t* C= container_of(p->p_child.next, pcb_t, p_sib);
        //rimozione figlio dalla lista
        list_del(p->p_child.next);
        return C;
    }
}

//rendi la pcb puntata da p non più figlia di suo padre e ritornalo, se p non ha padre ritorna NULL
pcb_t *outChild(pcb_t *p) {
    if(p->p_parent==NULL)
        return NULL;
    else{
        struct list_head *pos;
        list_for_each(pos, &p->p_parent->p_child){
            pcb_t *current= container_of(pos,pcb_t, p_sib);
            if(current == p){
                list_del(pos);
                return p;
            }
        }
        return NULL;
    }
}



