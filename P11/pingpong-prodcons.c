
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

task_t      p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga ;
int item = 0;
int buffer[4];
int pos = 0;

void produtor(void * arg) {
	while (1)
	{
		if(pos == 4){
			pos = 0;
		}
		task_sleep (1000);
		item =  (rand() % 100);
		sem_down(&s_vaga);
		sem_down(&s_buffer);
		buffer[pos + 1] = item;
		printf ("%s produziu (%d)\n", (char *) arg, item) ;
		sem_up (&s_buffer);
		sem_up (&s_item);
	}
}

void consumidor(void * arg) {
	while (1)
	{
		if(pos == 4){
			pos = 0;
		}
		sem_down (&s_item);
		sem_down (&s_buffer);
		sem_up (&s_buffer);
		sem_up (&s_vaga);
		printf ("%s consumiu (%d)\n", (char *) arg, buffer[pos + 1]) ;
		pos ++;
		task_sleep (1000);
	}
}

int main (int argc, char *argv[]) {

	printf ("main: inicio\n") ;

    ppos_init () ;

    sem_create (&s_buffer, 1) ;
    sem_create (&s_item, 0) ;
	sem_create (&s_vaga, 5) ;

    task_create (&p1, produtor, " p1 ") ;
    task_create (&p2, produtor, " p2 ") ;
    task_create (&p3, consumidor, " p3 ") ;
    task_create (&c1, consumidor, "                   c1 ") ;
    task_create (&c2, consumidor, "                   c2 ") ;

	task_join (&p1) ;

    // destroi semaforos
    sem_destroy (&s_buffer) ;
    sem_destroy (&s_item) ;
	sem_destroy (&s_vaga) ;


	// aguarda a2, b1 e b2 encerrarem
	task_join (&c1) ;
	task_join (&p2) ;
	task_join (&c2) ;
	task_join (&p3) ;

	printf ("main: fim\n") ;
	task_exit (0) ;

	exit (0) ;

}
