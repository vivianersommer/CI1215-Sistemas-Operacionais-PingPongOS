#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

task_t *scheduler(task_t *tarefasUser){
   task_t *aux = tarefasUser;
   printf ("%d\n",aux->id);
	aux = aux->next;
	while (aux != tarefasUser){
      printf ("%d\n",aux->id);
		aux = aux->next;
	} 
   return tarefasUser->next; 
}

void dispatcher (task_t *tarefasUser) {
   while(&tarefasUser->next != NULL){
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){
         task_switch (prox);
         switch (prox->status){
            case (0):
               queue_append ((queue_t **) &tarefasUser,  (queue_t*) &prox) ;
               break;
            case (1):
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) &prox) ;
               break;
            case (2):
               tarefasUser = tarefasUser->next;
               break;
            default:
               break;
         }
      }
   }
   task_exit(0);
}
