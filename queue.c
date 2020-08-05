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

    if(*queue == NULL){
        elem->next = elem;
        elem->prev = elem;
		*queue = elem;
        return;
    }
    else{
        queue_t *primeiro = *queue;
        queue_t *ultimo = primeiro->prev;
        ultimo->next = elem;
        elem->prev = ultimo;
        elem->next = primeiro;
        primeiro->prev = elem;
        return;
    }
}

queue_t *queue_remove (queue_t **queue, queue_t *elem){
	
	if (*queue == NULL && elem == NULL){
		return NULL;
	}

	elem->prev->next = elem->next;
    elem->next->prev = elem->prev;

	return *queue;
}

int queue_size(queue_t *queue){
	

	if (queue == NULL ) {
		return 0;
	}

	queue_t *primeiro = queue;
	queue_t *aux = primeiro;

	int i = 1;
	while ( aux->next != primeiro ){
		aux = aux->next;	
		i++;
	}
	
	return i;

}

void queue_print(char *name, queue_t *queue, void print_elem (void*) ){

	if (queue == NULL ) {
		return;
	}

	queue_t *primeiro = queue;
	queue_t *aux = primeiro;

	while ( aux->next != primeiro->next ){
		aux = aux->next;	
		print_elem( aux );		
	}
	return;		


}
