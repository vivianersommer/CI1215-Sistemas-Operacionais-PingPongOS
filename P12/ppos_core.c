#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "ppos.h"
#include "ppos_data.h"

#define DEBUG
#define STACKSIZE 32768	
#define N 100

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer ;
struct itimerval ZeraTimer = {0} ;

char *stack ;
int i=0 , premp;
int quantum ; 
int lock = 0 ;
unsigned int relogio;
task_t ContextMain, *ContextAtual ,*tarefasUser, Dispatcher , *tarefasNanando;

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
    premp = 1;
    tarefasUser = NULL;
    tarefasNanando = NULL;
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
        ContextMain.tarefasSuspensas = NULL;
    }
    else{
        perror ("Erro na criação da pilha: ");
        exit (-1);
    }
    task_setprio(&ContextMain,0);

    ContextMain.next = NULL;
    ContextMain.prev = NULL;

    ContextAtual = &ContextMain;

    Dispatcher.status = 1;
    Dispatcher.tipoTarefa = 0; // tarefa de sistema

    //Cria tarefa Main
    task_create(&ContextMain, NULL , 0);

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
        task->tarefasSuspensas = NULL;
        task->horaAcordar = 0;
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

    if(task->id == 0){
        #ifdef DEBUG
        printf ("task_create: criou tarefa %d - MAIN\n", task->id) ;
        #endif
    }else if(task->id == 1){
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
    if(ContextoAntigo->status != 2){
        ContextoAntigo->status = 1;
    }
    ContextAtual->status = 0;

    // #ifdef DEBUG
    // printf ("task_switch: trocando contexto %d para %d\n",ContextoAntigo->id, task->id) ;
    // #endif

    swapcontext (&ContextoAntigo->context,&task->context) ;
    return task_id(); 
}

void task_exit (int exit_code){

    task_t *aux = ContextAtual->tarefasSuspensas;
    if(aux != NULL){
        task_t *trocar = aux;
        aux->status = 0;
        aux = aux->next;    
        queue_remove ( ( queue_t** ) &(ContextAtual->tarefasSuspensas), (queue_t*) trocar) ; 
        trocar->next = NULL;
        trocar->prev = NULL;
        queue_append ( ( queue_t** ) &tarefasUser, (queue_t*) trocar ) ;
    }

    ContextAtual->status = 2;
    ContextAtual->exitcode = exit_code;
    ContextAtual->horarioFim = systime();

    
    #ifdef DEBUG
    printf ("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
            ContextAtual->id, (ContextAtual->horarioFim - ContextAtual->horarioInicio),
            ContextAtual->horarioProcessador , ContextAtual->ativacoes) ;
    #endif

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
    int processadorInicioDispatcher = systime(); 
    while( queue_size( (queue_t*)tarefasUser) > 0 || queue_size( (queue_t*)tarefasNanando) > 0) { //analiza se existe algum elemento na fila de tarefas prontas
        if(tarefasUser != NULL){
            task_t *prox = scheduler(tarefasUser);
            if(prox != NULL){ //se o scheduler retorna uma tarefa, retira ela da fila e realiza task_switch
                quantum = 20;
                prox->ativacoes = prox->ativacoes + 1;
                int processadorInicio = systime();
                queue_remove ((queue_t**) &tarefasUser, (queue_t*) prox) ;
                task_switch (prox);
                Dispatcher.ativacoes = Dispatcher.ativacoes + 1;
                prox->horarioProcessador = prox->horarioProcessador + (systime() - processadorInicio);
            }
        }
        acordaTarefas();
    }
    Dispatcher.horarioProcessador = Dispatcher.horarioProcessador + (systime() - processadorInicioDispatcher);
    task_exit(0);  //quando a fila esvazia, encerra o dispatcher, pois ele também é uma tarefa
}

void tratador (int signum)
{
    relogio ++;
    if(ContextAtual->tipoTarefa == 1){ //apenas faz preempção quando é tarefa de usuário
        quantum --;
        if(quantum == 0 && premp == 1){ //troca de contexto quando acaba o quantum
            premp = 0;
            queue_append((queue_t**)&tarefasUser, (queue_t*)ContextAtual);
            premp = 1;
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

    premp = 0;
	
	if ( task == NULL ){
		return -1;
	}

    if(task->status == 2){
        return task->exitcode;
    }

    //suspende tarefa atual 
    ContextAtual->status = 1;
    queue_remove ( ( queue_t** ) &tarefasUser, (queue_t*) ContextAtual ) ;
    queue_t* context = (queue_t*) ContextAtual;
    if(ContextAtual!=NULL){
	    context->next = NULL;
	    context->prev = NULL;
    }

    queue_append ( ( queue_t** ) &task->tarefasSuspensas , ( context )) ;

    premp = 1;

    task_yield();

    return task->exitcode;
}

void task_sleep (int t){
    if(ContextAtual == NULL){
        return;
    }

    premp = 0;

    task_t *trocar = ContextAtual;
    ContextAtual->horaAcordar = systime() + t;
    ContextAtual->status = 1;
    queue_remove ( ( queue_t** ) &tarefasUser, (queue_t*) trocar) ; 
    trocar->next = NULL;
    trocar->prev = NULL;
    queue_append ( ( queue_t** ) &tarefasNanando, (queue_t*) trocar ) ;

    premp = 1;
    
    task_yield();
}

void acordaTarefas(){
    if(tarefasNanando == NULL){
        return;
    }
    premp = 0;
    task_t *aux = tarefasNanando;
    do{
        if(aux->horaAcordar <= systime()){
            task_t *trocar = aux;
            aux->status = 0;
            aux = aux->next;    
            queue_remove ( ( queue_t** ) &tarefasNanando, (queue_t*) trocar) ; 
            trocar->next = NULL;
            trocar->prev = NULL;
            queue_append ( ( queue_t** ) &tarefasUser, (queue_t*) trocar ) ;
        }
        else{
            aux = aux->next;    
        }
    } while((tarefasNanando!= NULL && aux!= NULL) && tarefasNanando != aux );
    premp = 1;
    return;
}
 
void enter_cs (int *lock)
{
  while (__sync_fetch_and_or (lock, 1)) ;
}
 
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}

int sem_create (semaphore_t *s, int value){ //acho q essa tá ok
    if(s == NULL){
        return -1;
    }
    enter_cs (&lock) ;
    s->counter = value;
    s->Suspensas = NULL;
    printf("Criado semáforo com value %d\n",s->counter);
    leave_cs (&lock) ;
    return 0;
}

int sem_down (semaphore_t *s){
    if(s == NULL){
        return -1;
    }
    enter_cs (&lock) ;
    s->counter -- ;
    if(s->counter <0){
        task_t *trocar = ContextAtual;
        trocar->next = NULL;
        trocar->prev = NULL;
        queue_append((queue_t **) &(s->Suspensas),(queue_t *) trocar);
        leave_cs (&lock) ;
        task_yield();
    }
    else{
        leave_cs (&lock) ;
    }
    return 0;
}

int sem_up (semaphore_t *s){
    if(s == NULL){
        return -1;
    }
    enter_cs (&lock) ;
    s->counter ++ ;
    if(s->counter <=0){
        task_t *trocar = s->Suspensas;
        queue_remove((queue_t **) &(s->Suspensas),(queue_t *) trocar);
        if(trocar!=NULL){
            trocar->next = NULL;
            trocar->prev = NULL;
             queue_append((queue_t **) &(tarefasUser),(queue_t *) trocar);
        }
    }
    leave_cs (&lock) ;
    return 0;
}

int sem_destroy (semaphore_t *s){
    enter_cs (&lock) ;
    if(s == NULL){
        return -1;
    }
    task_t *aux = s->Suspensas;
    if(aux != NULL){
        do{
            task_t *trocar = aux;
            aux->status = 0;
            aux = aux->next;    
            queue_remove ( ( queue_t** ) &(s->Suspensas), (queue_t*) trocar) ; 
            trocar->next = NULL;
            trocar->prev = NULL;
            queue_append ( ( queue_t** ) &tarefasUser, (queue_t*) trocar ) ;
        } while((s->Suspensas!= NULL && aux!= NULL) && s->Suspensas != aux );
    }
    s = NULL;
    leave_cs (&lock) ;
    return 0;
}

int mqueue_create (mqueue_t *queue, int max, int size) {  //ok
    if(queue == NULL){
        return -1;
    }

    queue->conteudo = malloc (size * max);
    queue->inicio = 0;
    queue->fim = 0;
    queue->tamanhoMax = max;
    queue->tamanhoMomento= 0;
    queue->sizeOf = size;
    sem_create (&queue->s_buffer, 1) ;
    sem_create (&queue->s_item, 0) ;
	sem_create (&queue->s_vaga, 5) ;

    return 0;
}

int mqueue_send (mqueue_t *queue, void *msg) { //1 
    if(queue == NULL){
        return -1;
    }
    premp = 0;
    sem_down(&queue->s_vaga);
    sem_down(&queue->s_buffer);
    memcpy(queue->conteudo + (queue->fim)*(queue->sizeOf), msg , queue->sizeOf);
    queue->tamanhoMomento ++;
    queue->fim = (queue->fim + 1) % queue->tamanhoMax;
	sem_up (&queue->s_buffer);
	sem_up (&queue->s_item);
	premp = 1;
    return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg) { //2
    if(queue == NULL){
        return -1;
    }
    premp = 0;
    sem_down(&queue->s_vaga);
    sem_down(&queue->s_buffer);
    memcpy(msg, queue->conteudo + (queue->inicio)*(queue->sizeOf) , queue->sizeOf);
    queue->inicio = (queue->inicio + 1) % queue->tamanhoMax;
	queue->tamanhoMomento--;
	sem_up(&queue->s_buffer);
	sem_up(&queue->s_vaga);
    premp = 1;
    return 0;
}

int mqueue_destroy (mqueue_t *queue) { //ok
    if(queue == NULL){
        return -1;
    }
    queue->inicio = 0;
    queue->fim = 0;
    queue->tamanhoMax = 0;
    queue->tamanhoMomento = 0;
    sem_destroy (&queue->s_buffer) ;
    sem_destroy (&queue->s_item) ;
	sem_destroy (&queue->s_vaga) ;
    free(queue->conteudo);
    queue = NULL;
    return 0;
}

int mqueue_msgs (mqueue_t *queue) {
    if(queue == NULL){
        return -1;
    }
    return queue->tamanhoMomento;
}