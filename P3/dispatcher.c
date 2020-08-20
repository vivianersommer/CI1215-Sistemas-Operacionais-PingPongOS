#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

task_t *scheduler(task_t *tarefasUser){
   if(tarefasUser != NULL){
	   if (tarefasUser->next == tarefasUser){		   
		printf(" tarefa %d ", tarefasUser->id );
	  
	      return tarefasUser; 
   	   }else {
	   	return tarefasUser->next;	  
	   }
   }
   return NULL; 
}

void dispatcher (task_t *tarefasUser) {
   while(tarefasUser != NULL){
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){
         task_switch (prox);
         switch (prox->status){
            case (0):

               tarefasUser = tarefasUser->next;
	       //queue_append ((queue_t **) &tarefasUser,  (queue_t*) &prox) ;
         	task_switch (tarefasUser);
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
