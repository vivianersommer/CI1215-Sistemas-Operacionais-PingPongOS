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
   task_t tarefa ;
} filaTarefa ;


task_t scheduler(filaTarefa *tarefasUser){
   return tarefasUser->next->tarefa;
}

void dispatcher (filaTarefa *tarefasUser) {
   while(tarefasUser > 0){
      task_t proxima = scheduler(tarefasUser);
      if(proxima.id >= 0){
         task_switch (&proxima);
         switch (proxima.status){
            case (0):
               queue_append ((queue_t **) &tarefasUser,  (queue_t*) &proxima) ;
               break;
            case (1):
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) &proxima) ;
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
