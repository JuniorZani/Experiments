/*******************************************************************************
* Este programa esta baseado no segundo experimento do curso sobre tempo real 
* do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #2: Multi-Tasking, Measuring Drift
*
*    Programmer: Eric Sorton
*          Date: 1/27/97
*           For: MSE599, Special Topics Class
*
*       Purpose: When a basic sleep call is used to determine the precise time
*                when an action will occur the problem of drift occurs. 
*                The measurement of time is imprecise.  Similarly, the time in 
*                which the sleep call returns is imprecise.  Over time, this 
*                will cause the ocurrence of time to drift.  Just as if a clock 
*                loses 1 second every day, over one day, it is significant, but 
*                over a year, it loses 365 seconds, which is over 6 minutes.  
*                This is an example of drift.
*
*       Proposito: Quando uma chamada b�sica sleep e usada para determinar o
*                instante exato em que alguma acao vai ocorrer, ocorre o problema
*                do desvio. A medicao de tempo e imprecisa. Similarmente, o tempo
*                que demora o retorno da chamada sleep tambem e impreciso. Ao
*                longo do tempo, isto ocasionara um desvio de tempo. Algo como se
*                um relogio perdesse um segundo a cada dia. Ao longo de um dia, 
*                essa diferenca e insignificante, mas, ao longo de um ano, sao 
*                perdidos 365 segundos, o que e superior a 6 minutos. Este e um
*                exemplo de desvio.
*
*******************************************************************************/

/*
 * Includes Necessarios, verifique se as bibliotecas no diretorio sys/ estao
 * lah. Caso nao estejam, verifique onde estao e altere o include
 */

#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>			/* for gettimeofday() and fork() */
#include <stdio.h>			/* for printf() */
#include <stdlib.h>			/* for exit(0) */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */

#define NO_OF_CHILDREN	5


int main( int argc, char *argv[] )
{
	if(argc < 2){
		printf("\nPoucos argumentos\n");
		return 1;
	}else{
		if(atoi(argv[1])%200 != 0){
			printf("\nSleep time deve ser multiplo de 200\n");
			return 1;
		}
	} 
		
    float drift;
    int count;
    int child_no;
	int rtn = 1;
	int sons[5];


	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
			rtn = fork();
			sons[count] = rtn;

		} else {
			break;
		}
	}


	if( rtn == 0 ) {

		child_no = count;

		char child_no_char[20];
		sprintf(child_no_char, "%d", child_no);

		char * args[] = {"./execFilho", child_no_char, argv[1], NULL};

		execvp(args[0], args);

	} else {
		for( count = 0; count < NO_OF_CHILDREN; count++ ) {
			kill(sons[count], SIGKILL);
			wait(NULL);
		}
	}

	exit(0);
}