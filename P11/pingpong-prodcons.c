
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"

task_t      p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga ;
int item = 0;
int buffer[4];
int primeiro = 0;
int ultimo = -1;
int capacidade = 5;

void produtor(void * arg) {
	while (1)
	{
		if(ultimo == (capacidade-1)){ 
			ultimo = -1;
		}
		if(s_buffer.counter == 5){
			continue;
		}
		else{
			task_sleep (1000);
			item =  (rand() % 100);
			sem_down(&s_vaga);
			sem_down(&s_buffer);
			ultimo++;
			s_buffer.counter ++;
			buffer[ultimo] = item;
			printf ("%s produziu (%d)\n", (char *) arg, item) ;
			sem_up (&s_buffer);
			sem_up (&s_item);
		}
	}
}

void consumidor(void * arg) {
	while (1)
	{
		if(ultimo == primeiro){
			continue;
		}
		if(s_buffer.counter == 0){
			continue;
		}
		else{
			sem_down (&s_item);
			sem_down (&s_buffer);
			int teste = primeiro++;
			printf ("%s consumiu (%d)\n", (char *) arg, buffer[teste]) ;
			if(primeiro == capacidade){
				primeiro = 0;
			}
			s_buffer.counter--;
			sem_up (&s_buffer);
			sem_up (&s_vaga);
			task_sleep (1000);
		}
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
    task_create (&p3, produtor, " p3 ") ;
    task_create (&c1, consumidor, "                   c1 ") ;
    task_create (&c2, consumidor, "                   c2 ") ;

	task_join (&p1) ;


    // destroi semaforos
    sem_destroy (&s_buffer) ;
    sem_destroy (&s_item) ;
	sem_destroy (&s_vaga) ;

	printf ("main: fim\n") ;
	task_exit (0) ;

	exit (0) ;

}
