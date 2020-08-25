#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"
#include "ppos_data.h"

#define DEBUG
#define STACKSIZE 32768	
#define N 100

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer ;

char *stack ;
int i=1;
task_t ContextMain, *ContextAtual ,*tarefasUser, Dispatcher;

/*  P4

    Alunas: 
    
        GRR20182564 Viviane da Rosa Sommer
        GRR20185174 Luzia Millena Santos Silva

    Arquivo pingpong.c:

        Implementada as seguintes funções:
            1 -  ppos_init  - Inicializa o sistema
            2 -  task_create - Cria uma nova tarefa
            3 -  task_switch  - Transfere o processador para outra tarefa
            4 -  task_exit - Termina a tarefa corrente
            5 -  task_id - Informa o identificador da tarefa corrente
            6 -  task_yield - Faz a troca de contexto para o dispatcher
            7 -  task_setprio - Ajusta a prioridade estática
            8 -  task_getprio - Recebe a prioridade estática
            9 -  scheduler  - Escolhe qual a próxima tarefa a ser executada
            10 - dispatcher - Realiza a troca de contexto entre tarefas **(ela também é uma tarefa)

    Arquivo ppos_data.h:

        Adicionado na estrutura task_t os seguintes campos:
            1 -  status : estado da tarefa
            2 -  prioridadeEstatica : nível de prioridade estatica da tarefa
            3 -  prioridadeDinamica : nível de prioridade dinamica da tarefa 

    Compilado com : cc -Wall queue.c pingpong.c pingpong-scheduler.c 

    Necessário ter os seguintes arquivos na pasta: 

            1 -  pingpong.c
            2 -  pingpong-scheduler.c
            3 -  ppos.h
            4 -  ppos_data.h
            5 -  queue.c
            6 -  queue.h

*/

void ppos_init (){

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;

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
        ContextMain.tipoTarefa = 1; //tarefa de usuário
    }
    else{
        perror ("Erro na criação da pilha: ");
        exit (-1);
    }
    task_setprio(&ContextMain,0);
    ContextMain.next = NULL;
    ContextMain.prev = NULL;
    ContextAtual = &ContextMain;

    #ifdef DEBUG
    printf ("ppos_init: criou tarefa %d - MAIN \n", ContextAtual->id) ;
    #endif

    Dispatcher.status = 1;
    Dispatcher.tipoTarefa = 0; // tarefa de sistema
    //Cria tarefa dispatcher
    task_create(&Dispatcher, dispatcher, NULL);

    temporizador();
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
        if(task->id != 1){
            task->tipoTarefa = 1; //tarefa de usuário
        }
    }
    else
    {
        perror ("Erro na criação da pilha: ") ;
        return (-1) ;
    }

    queue_t* context = (queue_t*) task;
    context->next = NULL;
    context->prev = NULL;
    task_setprio(task,0);
    queue_append ((queue_t **) &tarefasUser,  context) ;

    makecontext (&task->context, (void*)(*start_routine), 1, arg);
    if(task->id == 1){
        #ifdef DEBUG
        printf ("task_create: criou tarefa %d - DISPACHER\n", task->id) ;
        #endif
    }
    else{
        #ifdef DEBUG
        printf ("task_create: criou tarefa %d\n", task->id) ;
        #endif
    }

    return task_id();     
}

int task_switch (task_t *task){

    task_t *ContextoAntigo;
    ContextoAntigo = ContextAtual;
    ContextAtual = task;
    (ContextoAntigo)->status = 1;
    ContextAtual->status = 0;

    #ifdef DEBUG
    printf ("task_switch: trocando contexto %d para %d\n",ContextoAntigo->id, task->id) ;
    #endif

    swapcontext (&ContextoAntigo->context,&task->context) ;
    return task_id(); 
}

void task_exit (int exit_code){

    // #ifdef DEBUG
    // printf ("task_exit: tarefa %d sendo encerrada\n", ContextAtual->id) ;
    // #endif
    
    ContextAtual->status = 2;
    task_switch(&Dispatcher);
    task_switch(&ContextMain);
}

int task_id (){
    return (int) *(&ContextAtual->id);
}

void task_yield(){  //Realiza a troca de contexto para o dispatcher
    ContextAtual->status = 1;
    task_switch(&Dispatcher);
}

void task_setprio (task_t *task, int prio){
    if(task && prio > -20 && prio < 20 ){ // Analiza se o tarefa existe, e se o valor passado está dentro dos limites 
        task->prioridadeEstatica = prio;
        task->prioridadeDinamica = prio;
    }
    else{
        perror ("Erro ao setar a prioridade da tarefa: ");
        exit (-1);
    }
}

int task_getprio (task_t *task){
    if(task == NULL){  //se não for enviada uma tarefa, retorna o valor da prioridade do Contexto Atual
        return (int) *(&ContextAtual->prioridadeEstatica);
    }
    return task->prioridadeEstatica;
}

task_t *scheduler(task_t *tarefasUser){
   task_t *prox = tarefasUser;
   task_t *aux = tarefasUser->next;

   while(aux != tarefasUser){
      if(aux->prioridadeDinamica <= prox->prioridadeDinamica){
         prox = aux; //coloca a prox como a que tem a maior prioridade dinamica
      }
      aux = aux->next;
   }
   
   aux = tarefasUser;
   while(tarefasUser->next != aux){
      if(tarefasUser->id != prox->id){
         tarefasUser->prioridadeDinamica = tarefasUser->prioridadeDinamica - 1; //envelhece a não prioritaria
      }
      tarefasUser = tarefasUser->next;
   }

   prox->prioridadeDinamica = prox->prioridadeEstatica; //a prioridade dinamica volta ao valor original da prioridade estatica
	return prox;
}

void dispatcher () {   
   while( queue_size( (queue_t*)tarefasUser) > 1) { //analiza se existe algum elemento na fila de tarefas prontas
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){ //se o scheduler retorna uma tarefa, retira ela da fila e realiza task_switch
         queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
         task_switch (prox);
         switch (prox->status){  //análize dos status das tarefas
            case (0): //PRONTA
               queue_append((queue_t**) &tarefasUser, (queue_t*) prox);
               break;
            case (1):  //SUSPENSA
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
               queue_append((queue_t**) &tarefasUser, (queue_t*) prox);
               break;
            default:  //FINALIZADA
               queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
               break;
         }
      }
   }
   task_exit(0);  //quando a fila esvazia, encerra o dispatcher, pois ele também é uma tarefa
}

void tratador (int signum)
{
   printf ("Recebi o sinal %d\n", signum) ;
}

void temporizador(){
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 0 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 3 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 0 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 1 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }

    // laco vazio
    while (1) ;
}


