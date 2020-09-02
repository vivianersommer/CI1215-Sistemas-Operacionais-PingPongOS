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
int quantum ; 
unsigned int relogio;
task_t ContextMain, *ContextAtual ,*tarefasUser, Dispatcher;

/*  P6

    Alunas: 
    
        GRR20182564 Viviane da Rosa Sommer
        GRR20185174 Luzia Millena Santos Silva

    Arquivo ppos_core.c:

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
            11 - tratador - Analiza se o quantum já acabou, e jaz a preempção
            12 - temporizador - Ajusta o temporizador para fazer a preempção
            13 - systime - Função que retorna uma simulação de relógio
            14 - imprime_fila - Imprime a fila de forma organizada
	    15 - task_join - Possibilita a sincronização de tarefas
 
    Arquivo ppos_data.h:

        Adicionado na estrutura task_t os seguintes campos:
            1 -  status : estado da tarefa
            2 -  prioridadeEstatica : nível de prioridade estatica da tarefa
            3 -  prioridadeDinamica : nível de prioridade dinamica da tarefa 
            4 -  tipoTarefa :  0 = tarefa de sistema e 1 = tarefa de usuário
            5 -  horarioInicio : salva a hora q inicia a tarefa
            6 -  horarioFim : salva a hora q finaliza a tarefa 
            7 -  horarioProcessador : salva o tempo de execução da tarefa
            8 -  ativacoes : conta quantas vezes a tarefa trocou de contexto

        Adicionado as funções:
            1 -  void dispatcher () ;
            2 -  void temporizador() ;
            3 -  void imprime_fila(task_t *fila);
	    4 -  int task_join();
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
        ContextMain.horarioInicio = systime();
        ContextMain.ativacoes = 0;
        ContextMain.horarioProcessador = 0;
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

    makecontext (&ContextMain.context, (void*)(*main), 1, NULL);
    //Insere tarefa main na fila
    queue_append ((queue_t **) &tarefasUser, (queue_t *) (&ContextMain)) ;
    
//Cria tarefa Main
//task_create(&ContextMain, void (&main)(), NULL);
    
    Dispatcher.status = 1;
    Dispatcher.tipoTarefa = 0; // tarefa de sistema

    //Cria tarefa dispatcher
    task_create(&Dispatcher, dispatcher, NULL);
    queue_remove ((queue_t**) &tarefasUser, (queue_t*) &Dispatcher) ;

    // ajusta e ativa mecanismo de preempção por tempo
    temporizador();

    //Ativa dispatcher ao iniciar
    task_yield () ;
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
        task->horarioInicio = systime();
        task->ativacoes = 0 ;
        task->horarioProcessador = 0;
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

    makecontext (&task->context, (void*)(*start_routine), 1, arg);

    queue_append ((queue_t **) &tarefasUser,  context) ;

    // if(task->id == 1){
    //     #ifdef DEBUG
    //     printf ("task_create: criou tarefa %d - DISPACHER\n", task->id) ;
    //     #endif
    // }
    // else{
    //     #ifdef DEBUG
    //     printf ("task_create: criou tarefa %d\n", task->id) ;
    //     #endif
    // }

    return task_id();     
}

int task_switch (task_t *task){

    task_t *ContextoAntigo;
    ContextoAntigo = ContextAtual;
    ContextAtual = task;
    ContextoAntigo->status = 1;
    ContextAtual->status = 0;

    // #ifdef DEBUG
    // printf ("task_switch: trocando contexto %d para %d\n",ContextoAntigo->id, task->id) ;
    // #endif

    swapcontext (&ContextoAntigo->context,&task->context) ;
    return task_id(); 
}

void task_exit (int exit_code){
    
    ContextAtual->status = 2;
    ContextAtual->horarioFim = systime();

    #ifdef DEBUG
    printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
            ContextAtual->id, (ContextAtual->horarioFim - ContextAtual->horarioInicio),
            ContextAtual->horarioProcessador , ContextAtual->ativacoes) ;
    #endif

    task_t *aux = ContextAtual->tarefasSuspensas;
    if( aux!= NULL){
        aux = aux->next;
        //acorda tarefas suspensas devido a task_join()
        while ( aux != ContextAtual->tarefasSuspensas ){
            printf("oi\n");
            aux->status = 0;
            queue_remove ( ( queue_t** ) ContextAtual->tarefasSuspensas, (queue_t*) aux ) ;
            queue_t* context = (queue_t*) aux;
            if(ContextAtual!=NULL){
                context->next = NULL;
                context->prev = NULL;
            }
            queue_append ( ( queue_t** ) tarefasUser, ( queue_t* ) aux ) ;
            aux = aux->next;
        }
    }


    if(ContextAtual->id == 1){
        task_switch(&ContextMain);
    }else{
        task_switch(&Dispatcher);
    }
}

int task_id (){
    return (int) *(&ContextAtual->id);
}

void task_yield(){  //Realiza a troca de contexto para o dispatcher
    if(ContextAtual->id != 0){
        ContextAtual->status = 0;
        queue_append((queue_t**)&tarefasUser, (queue_t*)ContextAtual);
    }
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
   while( queue_size( (queue_t*)tarefasUser) > 0) { //analiza se existe algum elemento na fila de tarefas prontas
      task_t *prox = scheduler(tarefasUser);
      if(prox != NULL){ //se o scheduler retorna uma tarefa, retira ela da fila e realiza task_switch
        quantum = 20;
        prox->ativacoes = prox->ativacoes + 1;
        int processadorInicio = systime();
        // imprime_fila(tarefasUser);
        queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
        task_switch (prox);
        Dispatcher.ativacoes = Dispatcher.ativacoes + 1;
        prox->horarioProcessador = prox->horarioProcessador + (systime() - processadorInicio);
      }
   }
   task_exit(0);  //quando a fila esvazia, encerra o dispatcher, pois ele também é uma tarefa
}

void tratador (int signum)
{
    relogio ++;
    if(ContextAtual->tipoTarefa == 1){ //apenas faz preempção quando é tarefa de usuário
        quantum --;
        if(quantum == 0){ //troca de contexto quando acaba o quantum
            queue_append((queue_t**)&tarefasUser, (queue_t*)ContextAtual);
            task_switch(&Dispatcher);
        }
    }
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
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}

unsigned int systime () { //cálculo de tempo
    return relogio;
}


void imprime_fila(task_t *tarefasUser){ // função extra para imprimir o conteudo da fila

	printf("Saída gerada: ");
	if (tarefasUser == NULL ) {
		printf("[] \n");
		return;	
	}
	printf("[");
	task_t *aux = tarefasUser;
	printf(" %d ",aux->id);
	printf(" ");
	aux = aux->next;
	while (aux != tarefasUser){
	printf(" %d ",aux->id);
		aux = aux->next;
		printf(" ");
	} 
	printf("] \n");
	return;		
}


int task_join(task_t *task){
	if ( task == NULL ){
		return -1;
    }
	//suspende tarefa atual 
	ContextAtual->status = 1;
    queue_remove ( ( queue_t** ) &tarefasUser, (queue_t*) ContextAtual ) ;
    queue_t* context = (queue_t*) ContextAtual;
    if(ContextAtual!=NULL){
        context->next = NULL;
        context->prev = NULL;
    }
    queue_append ( ( queue_t** ) &task->tarefasSuspensas , ( queue_t* )( &ContextAtual )) ;
    task_yield();
	return task->id;
}
