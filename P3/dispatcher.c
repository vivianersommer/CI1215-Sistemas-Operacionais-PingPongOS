#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

typedef struct filaTarefa
{
   struct filaTarefa *prev ;  
   struct filaTarefa *next ;  
   task_t *tarefa ;
} filaTarefa ;


task_t *scheduler(filaTarefa *tarefasUser){
   if(tarefasUser->tarefa != NULL){
      task_t *tarefa = tarefasUser->next->tarefa;
      return tarefa;
   }
   return NULL;
}

void dispatcher (filaTarefa *tarefasUser) {
   while(tarefasUser->next != NULL){

      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){
	#ifdef DEBUG
	printf("entrou no while");
    	#endif
   
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
