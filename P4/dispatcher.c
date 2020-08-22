#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

/*

	GRR20182564 Viviane da Rosa Sommer
	GRR20185174 Luzia Millena Santos Silva

   Implementada as seguintes funções:
   scheduler  - Escolhe qual a próxima tarefa a ser executada
   dispatcher - Realiza a troca de contexto entre tarefas **(ela também é uma tarefa)

*/

extern task_t *tarefasUser;

task_t *scheduler(task_t *tarefasUser){

	return tarefasUser;
}

void dispatcher () {   
   while( queue_size( (queue_t*)tarefasUser) > 1) {
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){
         queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
	      //prox->status = 1;
         task_switch (prox);
         switch (prox->status){
            case (0):
               queue_append((queue_t**) &tarefasUser, (queue_t*) prox);
               break;
            case (1):
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
               queue_append((queue_t**) &tarefasUser, (queue_t*) prox);
               break;
            default:
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
               break;
         }
      }
   }
   task_exit(0);
}
