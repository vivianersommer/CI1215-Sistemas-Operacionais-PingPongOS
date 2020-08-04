#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

/*

    Implementar essas funções:
    queue_append - adicionar elemento
    queue_remove - remover elemento
    queue_size - retorna o tamanho da fila
    queue_print - imprime a fila

*/

void queue_append(queue_t **queue, queue_t *elem){

    if(queue == NULL){
		queue=elem;
        elem->next = elem;
        elem->prev = elem;
    }
    else{
        queue_t *primeiro = *queue;
        queue_t *ultimo = primeiro->prev;
        ultimo->next = elem;
        elem->prev = ultimo;
        elem->next = primeiro;
        primeiro->prev = elem;
     }
     
}

queue_t *queue_remove (queue_t **queue, queue_t *elem){

}

int queue_size(queue_t *queue){

}

void queue_print(char *name, queue_t *queue, void print_elem (void*) ){

}