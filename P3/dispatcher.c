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

   while( queue_size( (queue_t*)tarefasUser) > 1) {
      int tam = queue_size( (queue_t*)tarefasUser);
      task_t *prox = scheduler(tarefasUser);
      
      if(prox != NULL){
         queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
	      prox->status = 1;
         task_switch (prox);
         switch (prox->status){
            case (0):
               break;
            case (1):
               break;
            default:
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
               break;
            }
         }
   }
   task_exit(0);
}
