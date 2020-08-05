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
	*queue=elem;
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
	
	if ( *queue == NULL && elem == NULL ){
		return *queue;
	}

	
	queue_t *aux = *queue;
	queue_t *primeiro = *queue;

	//search for elem in queue
	while ( aux != elem ){
		if ( aux == primeiro ) {
			return *queue;
		}
		aux = aux->next;
	}


	queue_t *esq = elem->prev;      	
	queue_t *dir = elem->next;


	esq->next = dir;
	esq->prev = dir->prev->prev;

	dir->prev = esq;
	dir->next = esq->next->next;


	return *queue;

}

int queue_size(queue_t *queue){
	

	if ( queue == NULL ) {
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

	if ( queue == NULL ) {
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
