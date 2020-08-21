#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

task_t *scheduler(task_t *tarefasUser){

	/*task_t *aux = tarefasUser;
	aux = aux->next;
	while (aux != tarefasUser){
		printf("%d\n",aux->id);
		aux = aux->next;
	}
	
     	if( tarefasUser != NULL ){
		//Se tem apenas 1 tarefa eh o dispatcher
   		if ( tarefasUser->next == tarefasUser ){		   
			printf( " tarefa %d ", tarefasUser->id );
		        return NULL; 
   	   	}else { 
//			printf( " tarefa %d ", tarefasUser->id );
	   		return tarefasUser->next;	  
	   	}
   	}
   	return NULL; */
		task_t *aux = tarefasUser->next;
	return aux->next;
}

void dispatcher (task_t *tarefasUser) {
     #ifdef DEBUG
     printf ("dispatcher tarefa: %d ", tarefasUser->id ) ;
     #endif
    
   while( queue_size( (queue_t*)tarefasUser) > 0 ) {

      task_t *prox = scheduler(tarefasUser);
    
      if(prox != NULL){

         queue_remove ((queue_t**) &tarefasUser, (queue_t*) &prox) ;
	 prox->status = 1;
         task_switch (prox);

	 switch (prox->status){
            case (0):
		
	       //queue_append ((queue_t **) &tarefasUser,  (queue_t*) &prox) ;
	       //task_yield();
	       //queue_append ((queue_t **) &tarefasUser,  (queue_t*) &prox) ;
               //task_switch (tarefasUser);
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
