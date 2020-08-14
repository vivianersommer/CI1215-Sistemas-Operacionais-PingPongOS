#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ppos.h"
#include "ppos_data.h"

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

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;

}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {

}

int task_switch (task_t *task){

}

void task_exit (int exit_code){

}

int task_id (){
    
}

