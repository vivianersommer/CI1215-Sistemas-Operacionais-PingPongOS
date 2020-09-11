#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"
#define DEBUG

/*

	GRR20182564 Viviane da Rosa Sommer
	GRR20185174 Luzia Millena Santos Silva

    Implementada as seguintes funções:
    queue_append - adicionar elemento
    queue_remove - remover elemento
    queue_size - retorna o tamanho da fila
    queue_print - imprime a fila

*/

void queue_append(queue_t **queue, queue_t *elem){ 
	
    //verifica se elemento a inserir esta isolado	
    if ( elem->next != NULL && elem->prev != NULL ){
		return;
    }

    //inserçao em fila vazia
    if( *queue == NULL ){
        elem->next = elem->prev = elem;
		*queue = elem;
    }
    else {
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
//	printf("entrei\n");

	if ( *queue == NULL ){
		return NULL;
	}
	//verifica se elemento pertence a fila
	queue_t *aux = (*queue)->next;
	while ( aux != elem ) {
		if( aux->next == ( *queue )->next ) {
			return NULL;
		}
		aux = aux->next;
	} 

	// A e C sao elementos adjacentes ao que sera removido
	queue_t *A = elem->prev;      	
	queue_t *C = elem->next;

	//se fila tem apenas um elemento, este eh removido
	if ( A == elem && C == elem ) {

		*queue = NULL;
		elem->next = NULL;
		elem->prev = NULL;
		return elem;
	}
	
	//ajuste de ponteiros para remover elemento
    	A->next = C;
		C->prev = A;

    	elem->next = NULL;
    	elem->prev = NULL;  

	//se o elemento removido eh a cabeca, ajusta ponteiro da mesma
    	if ( elem == *queue ){
        	*queue = C;
    	}
	return elem;
}

int queue_size(queue_t *queue){ 

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
	printf(" ");
	aux = aux->next;
	while (aux != queue){
		print_elem(aux);
		aux = aux->next;
		printf(" ");
	} 
	printf("] \n");
	return;		


}
