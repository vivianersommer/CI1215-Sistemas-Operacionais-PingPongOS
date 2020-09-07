// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas

// Estrutura que define um Task Control Block (TCB)

/*  P4 

    Alunas: 
    
        GRR20182564 Viviane da Rosa Sommer
        GRR20185174 Luzia Millena Santos Silva

    Arquivo ppos_data.h:

        Adicionado na estrutura task_t os seguintes campos:
            1 -  status : estado da tarefa
            2 -  prioridadeEstatica : nível de prioridade estatica da tarefa
            3 -  prioridadeDinamica : nível de prioridade dinamica da tarefa 

    Necessário ter os seguintes arquivos na pasta: 

            1 -  pingpong.c
            2 -  pingpong-scheduler.c
            3 -  ppos.h
            4 -  ppos_data.h
            5 -  queue.c
            6 -  queue.h

*/

typedef enum Status
{
  Pronta, //0
  Suspensa,  //1
  Terminada  //2
} Status; 

typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   Status status; //estado da tarefa
   int prioridadeEstatica; // nível de prioridade estatica da tarefa 
   int prioridadeDinamica; // nível de prioridade dinamica da tarefa 
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

void dispatcher () ;

void temporizador() ;

void imprime_fila(task_t *fila);

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

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
  // preencher quando necessário
} mqueue_t ;

#endif
