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

void queue_append(queue_t **queue, queue_t *elem){ // - OK

	if(elem->next != NULL || elem->prev != NULL){
		return;
	}

    if(*queue == NULL){
        elem->next = elem;
        elem->prev = elem;
		*queue = elem;
    }
    else{
        queue_t *primeiro = *queue;
	
        queue_t *ultimo = primeiro->prev;
        ultimo->next = elem;
        elem->prev = ultimo;
        elem->next = primeiro;
        primeiro->prev = elem;
    }
    return;
}

queue_t *queue_remove (queue_t **queue, queue_t *elem){
	
		if ( *queue == NULL){
			return *queue;
		}

		queue_t *aux = (*queue)->next;
		while (aux != elem){
			if(aux->next == (*queue)->next){
				return NULL;
			}
			aux = aux->next;
		} 

		queue_t *A = elem->prev;      	
		queue_t *C = elem->next;

		if ( A == elem && C == elem ) {
			*queue = NULL;
			elem->next = NULL;
			elem->prev = NULL;  
			return NULL;
		}

    	A->next = C;
		C->prev = A;

    	elem->next = NULL;
    	elem->prev = NULL;  

    	if ( elem == *queue ){
        	*queue = C;
    	}

	return *queue;
}

int queue_size(queue_t *queue){ // - OK

	if (queue == NULL ) {
		return 0;
	}

	queue_t *aux = queue;

	int i = 1;
	while ( aux->next != queue ){
		aux = aux->next;	
		i++;
	}
	
	return i;
}

void queue_print(char *name, queue_t *queue, void print_elem (void*) ){

	printf("Saída gerada: ");
	if (queue == NULL ) {
		printf("[] \n");
		return;	
	}

	printf("[");
	queue_t *aux = queue;
	print_elem(aux);
	aux = aux->next;
	while (aux != queue){
		print_elem(aux);
		aux = aux->next;
		printf(" ");
	} 
	printf("] \n");
	return;		


}
