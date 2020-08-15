#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"

#define STACKSIZE 32768	

task_t ContextMain,ContextAtual;

/*

	GRR20182564 Viviane da Rosa Sommer
	GRR20185174 Luzia Millena Santos Silva

    Implementada as seguintes funções:
    ppos_init  - Inicializa o sistema
    task_create - Cria uma nova tarefa
    task_switch  - Transfere o processador para outra tarefa
    task_exit - Termina a tarefa corrente
    task_id - Informa o identificador da tarefa corrente

*/

void ppos_init (){
    getcontext (&ContextMain.context) ;
    getcontext(&ContextAtual.context) ; 
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
   char *stack ;
   getcontext (&task->context) ;
   stack = malloc (STACKSIZE) ;
   if (stack)
   {
      (&task->context)->uc_stack.ss_sp = stack ;
      (&task->context)->uc_stack.ss_size = STACKSIZE ;
      (&task->context)->uc_stack.ss_flags = 0 ;
      (&task->context)->uc_link = 0 ;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      return (-1) ;
   }
   makecontext (&task->context, (void*)(*start_routine), 1, arg);
   return (int) (*task).id;        //task_id(); 
}

int task_switch (task_t *task){
    swapcontext (&ContextAtual.context, &task->context) ;
    return task_id(); 
}

void task_exit (int exit_code){
    task_switch(&ContextMain);
}

int task_id (){
    return (int) (&ContextMain)->id;
}
