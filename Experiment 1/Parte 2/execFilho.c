/*
 * NO_OF_ITERATIONS e o numero de vezes que vai se repetir o loop existente
 * em cada futuro processo filho. 
 */

#define NO_OF_ITERATIONS	1000

/*
 * MICRO_PER_SECOND define o numero de microsegundos em um segundo
 */

#define MICRO_PER_SECOND	1000000

/*
	 * start_time e stop_time conterao o valor de tempo antes e depois
	 * que as trocas de contexto comecem
         */

	struct timeval start_time;
	struct timeval stop_time;


#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>			/* for gettimeofday() and fork() */
#include <stdio.h>			/* for printf() */
#include <stdlib.h>			/* for exit(0) */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */



int main(int argc, char * argv[]){

    int args[] = {atoi(argv[1]), atoi(argv[2])};
    float drift;
    
    // printf("%s", argv[1]);

    /*
    * Primeiro, obtenho o tempo inicial.
    */
    gettimeofday( &start_time, NULL );


    /*
    * Este loop ocasiona a minha dormencia, de acordo com
    * SLEEP_TIME, tantas vezes quanto NO_OF_ITERATIONS
    */
    for(int count = 0; count < NO_OF_ITERATIONS; count++ ) {
        usleep(args[1]);
    }

    /*
    * Paraobter o tempo final
    */
    gettimeofday( &stop_time, NULL );

    /*
    * Calcula-se o desvio
    */
    drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
    drift += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND;


    printf("\nFilho #%d -- desvio total: %.5f -- desvio medio: %.5f\n", args[0], 
    (drift - NO_OF_ITERATIONS*args[1]/MICRO_PER_SECOND), 
    (drift - NO_OF_ITERATIONS*args[1]/MICRO_PER_SECOND)/NO_OF_ITERATIONS);
	
}