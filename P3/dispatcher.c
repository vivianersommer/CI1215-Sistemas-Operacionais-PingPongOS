#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

task_t *scheduler(task_t *tarefasUser){
	return tarefasUser;
}

void dispatcher (task_t *tarefasUser) {    
   while( queue_size( (queue_t*)tarefasUser) > 0 ) {
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){
         queue_remove ((queue_t**) &tarefasUser, (queue_t*) &prox) ;
	      prox->status = 1;
         task_switch (prox);
	   switch (prox->status){
            case (0):
		         tarefasUser = tarefasUser->next;
               break;
            case (1):
               tarefasUser = tarefasUser->next;
               break;
            case (2):
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) &prox) ;
               break;
            default:
               break;
         }
      }
   }
   task_exit(0);
}
