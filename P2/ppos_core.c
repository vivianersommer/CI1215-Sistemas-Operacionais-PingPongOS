#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"
#define DEBUG
#define STACKSIZE 32768	
#define N 100

char *stack ;
int i=1;
task_t ContextMain, *ContextAtual ,*tarefasUser;

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
    tarefasUser = NULL;
    char *stack;
    stack = malloc (STACKSIZE);
    if (stack){
        ContextMain.context.uc_stack.ss_sp = stack;
        ContextMain.context.uc_stack.ss_size = STACKSIZE;
        ContextMain.context.uc_stack.ss_flags = 0;
        ContextMain.context.uc_link = 0;
        ContextMain.id = 0;
        ContextMain.status = 0;
    }
    else{
        perror ("Erro na criação da pilha: ");
        exit (1);
    }
    ContextAtual = &ContextMain; 
    queue_t* context = (queue_t*) &ContextMain;
    context->next = NULL;
    context->prev = NULL;
    queue_append ((queue_t **) &tarefasUser,  context) ;
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
   getcontext (&task->context) ;
   char *stack; 
   stack = malloc (STACKSIZE) ;
   if (stack)
   {
      (&task->context)->uc_stack.ss_sp = stack ;
      (&task->context)->uc_stack.ss_size = STACKSIZE ;
      (&task->context)->uc_stack.ss_flags = 0 ;
      (&task->context)->uc_link = 0 ;
      *(&task->id) = i++;
      task->status = 0;
   }
   else
   {
      perror ("Erro na criação da pilha: ") ;
      return (-1) ;
   }
    getcontext(&ContextAtual->context);
    
    ContextAtual = &ContextMain; 

    queue_t* context = (queue_t*) task;
    context->next = NULL;
    context->prev = NULL;
    queue_append ((queue_t **) &tarefasUser,  context) ;

    makecontext (&task->context, (void*)(*start_routine), 1, arg);

    // #ifdef DEBUG
    // printf ("task_create: criou tarefa %d\n", task->id) ;
    // #endif
   return task_id();     
}

int task_switch (task_t *task){
    task_t *ContextoAntigo;
    ContextoAntigo = ContextAtual;
    ContextAtual = task;
    (ContextoAntigo)->status = 1;
    (ContextAtual)->status = 0;
    // #ifdef DEBUG
    // printf ("task_switch: trocando contexto %d para %d\n",ContextoAntigo->id, task->id) ;
    // #endif
    swapcontext (&ContextoAntigo->context,&task->context) ; 
    return task_id(); 
}

void task_exit (int exit_code){
    // #ifdef DEBUG
    // printf ("task_exit: tarefa %d\n sendo encerrada", ContextAtual->id) ;
    // #endif
    (ContextAtual)->status = 2;
    task_switch(&ContextMain);
}

int task_id (){
    return (int) *(&ContextAtual->id);
}

void task_yield(){
   dispatcher(tarefasUser);
}

