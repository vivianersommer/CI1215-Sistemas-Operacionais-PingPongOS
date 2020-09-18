// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas

// Estrutura que define um Task Control Block (TCB)

typedef enum Status
{
  Pronta, //0
  Suspensa,  //1
  Terminada,  //2
  Dormindo //3
} Status; 

typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   Status status; //estado da tarefa
   int prioridadeEstatica; // nível de prioridade estatica da tarefa 
   int prioridadeDinamica; // nível de prioridade dinamica da tarefa 
   int tipoTarefa; // 0 : tarefa de sistema e 1 : tarefa de usuário
   int horarioInicio; // salva a hora q inicia a tarefa
   int horarioFim; // salva a hora q finaliza a tarefa
   int horarioProcessador; // salva o tempo de execução
   int ativacoes; //quantas vezes trocou de contexto
   struct task_t *tarefasSuspensas;  //tarefas que foram suspensas por task_join()
   int horaAcordar;  //hora que a tarefa acorda (SE MANCA TAREFA)
   int exitcode; 
   // ... (outros campos serão adicionados mais tarde)
} task_t ;


// funções auxiliares usadas no ppos_core.c
void dispatcher () ;
//int main (int argc, char *argv[]);
void temporizador() ;

void imprime_fila(task_t *fila);

void acordaTarefas();

int main (int argc, char *argv[]);


// estrutura que define um semáforo
typedef struct
{
  struct task_t *Suspensas;
  int counter;
  // preencher quando necessário
} semaphore_t ;


int sem_create (semaphore_t *s, int value);

int sem_down (semaphore_t *s);

int sem_up (semaphore_t *s);

int sem_destroy (semaphore_t *s);


// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;



// estrutura que define uma fila de mensagens
typedef struct
{
  void** conteudo;
  int inicio;
  int fim;
  int tamanhoMomento;
  int tamanhoMax;
  int sizeOf;
  semaphore_t s_buffer;
  semaphore_t s_item;
  semaphore_t s_vaga;
} mqueue_t ;

#endif
