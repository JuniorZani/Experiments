/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #3: Shared Resources, Measureing Message Queue Transfer Time
*
*    Programmer: Eric Sorton
*          Date: 2/11/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to measure the time it takes
*                a message to be transfered across a message queue.  The
*                total time will include the time to make the call to msgsnd(),
*                the time for the system to transfer the message, the time
*                for the context switch, and finally, the time for the other
*                end to call msgrcv().
*
*                The algorithm for this program is very simple:
*
*                   o The parent creates the message queue
*                   o The parents starts two children
*                   o The first child will:
*                         - Receive a message on the queue
*                         - Call gettimeofday() to get the current time
*                         - Using the time in the message, calculate
*                              the difference and store it in an array
*                         - Loop (X number of times)
*	   			  - Display the results
*                   o The second child will:
*                         - Call gettimeofday() to get the current time
*                         - Place the time in a message
*                         - Place the message on the queue
*                         - Pause to allow the other child to run
*                         - Loop (X number of times)
*                   o The parent waits for the children to finish
*
* Traduzindo: 
*
*     Prop�sito: O prop�sito deste programa � a medicao do tempo que leva
*                uma mensagem para ser transferida por uma fila de mensagens.
*                O tempo total incluira o tempo para realizar a chamada 
*                msgsnd(), o tempo para o sistema transferir a mensagem, o
*                tempo para troca de contexto e, finalmente, o tempo para,
*                na outra ponta, ocorrer a chamada msgrcv().
*
*                O algoritmo para este programa e bastante simples:
*
*                   o O pai cria a fila de mensagens
*                   o O pai inicializa dois filhos
*                   o O primeiro filho:
*                         - Recebe uma mensagem pela fila
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Usando o tempo existente na mensagem, calcula
*                              a diferenca
*                         - Repete (numero X de vezes)
*				  - Exibe os resultados
*                   o O segundo filho:
*                         - Chama gettimeofday() para obter o tempo atual
*                         - Coloca o tempo em uma mensagem
*                         - Coloca a mensagem na fila
*                         - Realiza uma pausa para permitir a execucao do irmao
*                         - Repete (numero X de vezes)
*                   o O pai espera os filhos terminarem
*
*******************************************************************************/

/*
 * Includes Necessarios
 */
#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>			/* for printf() */
#include <stdlib.h>
#include <unistd.h>			/* for fork() */
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>			/* for wait() */
#include <sys/ipc.h>		/* for msgget(), msgctl() */
#include <sys/msg.h>		/* for msgget(), msgctl() */

#define NO_OF_CHILDREN 3
#define MESSAGE_QUEUE_ID	3102
#define MESSAGE_QUEUE_ID2	3103

/*
 * Programa principal 
 */
int main( int argc, char *argv[] )
{

	//Algumas variaveis necessarias

	int rtn;
	int count;
	int queue_id[2];
	int multiplier;
	key_t key;
	key_t key2;
	char charCount[20], charQueue1[20], charQueue2[20], charMulti[20];
	char *args[6];

	//Inicialização das variáveis
	key = MESSAGE_QUEUE_ID;
	key2 = MESSAGE_QUEUE_ID2;

	do{
	printf("Digite um valor de entre 1 a 10:");
	scanf("%d", &multiplier);
	}while(multiplier < 1 || multiplier > 10);
	
	//Inicializa dois filhos
	rtn = 1;
	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
			rtn = fork();
		} else {
			break;
		}
	}

	//Criação da primeira fila, que terá os valores de tempo que serão usados para os cálculos
	if( (queue_id[0] = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	//Criação da segunda fila, que terá os resultados
	if( (queue_id[1] = msgget(key2, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	/*
	 * Verifica se os processos são filhos, se sim, cria as filas e executa o arquivo com os processos filhos
     */
	if(rtn == 0){
		//Transforma as variáveis em String
		sprintf(charCount, "%d", count);
		sprintf(charQueue1, "%d", queue_id[0]);
		sprintf(charQueue2, "%d", queue_id[1]);
		sprintf(charMulti, "%d", multiplier);

		char * args[] = {"./filhos", charCount, charQueue1, charQueue2, charMulti, NULL};
		execvp(args[0], args);
	/*
		* Se não o pai espera a execução dos filhos
		*/
	} else {
		//Sou o pai aguardando meus filhos terminarem
		for(count = 0; count < NO_OF_CHILDREN; count++){
			wait(NULL);
		}
		//Remoção das filas após seu uso
		if( msgctl(queue_id[0],IPC_RMID,NULL) == -1 ) {
			fprintf(stderr,"Impossivel remover a fila!\n");
			exit(1);
		}

		if( msgctl(queue_id[1],IPC_RMID,NULL) == -1 ) {
			fprintf(stderr,"Impossivel remover a fila!\n");
			exit(1);
		}
	}
	exit(0);
}