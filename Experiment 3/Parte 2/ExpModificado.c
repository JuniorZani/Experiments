/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #5: Semaphores
*
*    Programmer: Eric Sorton
*          Date: 3/17/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to demonstrate how semaphores
*		 can be used to protect a critical region.  Its sole purpose
*		 is to print a character string (namely the alphabet) to the
*		 screen.  Any number of processes can be used to cooperatively
*		 (or non-cooperatively) print the string to the screen.  An
*		 index is stored in shared memory, this index is the index into
*		 the array that identifies which character within the string
*		 should be printed next.  Without semaphores, all the processes
*		 access this index simultaneously and conflicts occur.  With
*		 semahpores, the character string is displayed neatly to the
*		 screen.
*
*		 The optional semaphore protection can be compiled into the
*		 program using the MACRO definition of PROTECT.  To compile
*		 the semaphore protection into the program, uncomment the
*		 #define below.
*
*
*       Proposito: O proposito deste programa e o de demonstrar como semaforos
*		podem ser usados para proteger uma regiao critica. O programa exibe
*		um string de caracteres (na realidade um alfabeto). Um n�mero 
*		qualquer de processos pode ser usado para exibir o string, seja
*		de maneira cooperativa ou nao cooperativa. Um indice e armazenado
*		em memoria compartilhada, este indice e aquele usado para 
* 		identificar qual caractere deve ser exibido em seguida. Sem 
*		semaforos, todos os processos acessam esse indice concorrentemente 
*		causando conflitos. Com semaforos, o string de caracteres e exibido
*		de maneira correta (caracteres do alfabeto na ordem correta e apenas
*		um de cada caractere).
*
*		A protecao opcional com semaforo pode ser compilada no programa
*		usando a definicao de MACRO denominada PROTECT. Para compilar a
*		protecao com semaforo, retire o comentario do #define que segue.
*
*
*******************************************************************************/



//#define PROTECT



/*
 * Includes Necessarios 
 */
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <stdio.h>              /* for printf() */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */


#define SEM_KEY1	0x1243
#define SEM_KEY2	0x1244
#define SEM_KEY3	0x1245
#define SEM_KEY4	0x1246
#define SHM_KEY1	0x1432
#define SHM_KEY2	0x1433
#define SHM_KEY3	0x1434
#define TAM 66		//Tamanho da String
#define NO_OF_CHILDREN	10

#define EMP 0
#define EMC 1
#define LIVRE 2
#define OCUPA 3

int	g_sem_id[4];

int g_shm_buffer_id;
int g_shm_prod_id;
int g_shm_consu_id;

char *g_shm_buffer_addr;
int	*g_shm_prod_addr;
int	*g_shm_consu_addr;

struct sembuf g_sem_op1[4]; //Fecha
struct sembuf g_sem_op2[4]; //Abre

char g_letters_and_numbers[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";

void initializeIPCS();

void Producer(int count);
void produceLetter(int number, int count);

void Consumer();
void consumeLetter(int number);

void removeIPCS();

int main( int argc, char *argv[] ){
	int rtn;
	int count;
	int pid[NO_OF_CHILDREN];

	initializeIPCS();

	rtn = 1;
	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
				pid[count] = rtn = fork();
		} else {
				break;
		}
	}

    if( rtn == 0 ) {
		fprintf(stderr,"\nFilho %i comecou ...\n", count);
		if(count < 10){
			Producer(count);
		}else {
			Consumer();
		}

	} else {
		usleep(100000);
		for(count = 0; count < NO_OF_CHILDREN; count++){
			kill(pid[count], SIGKILL);
		}
		removeIPCS();
		fprintf(stderr,"\n");
		exit(0);
	}
}

void Producer(int count){
	struct timeval tv;
	int number;

	usleep(1000);

	while(1){
		if(gettimeofday(&tv,NULL) == -1){
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}

		number = ((tv.tv_usec / 47) % 4) + 1;

	//P(Livre)	
#ifdef PROTECT
	if(semop(g_sem_id[LIVRE], g_sem_op1, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso! Livre");
	    exit(1);
	}
#endif

	//P(Emp)
#ifdef PROTECT
	if(semop(g_sem_id[EMP], g_sem_op1, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso EMP!");
	    exit(1);
	}
#endif
	
		produceLetter(number, count);
	
	//V(Emp)
#ifdef PROTECT
	if(semop(g_sem_id[EMP], g_sem_op2, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel abrir o recurso EMP!");
		exit(1);
	}
#endif

	//V(Ocupa)
#ifdef PROTECT
	if(semop(g_sem_id[OCUPA], g_sem_op2, 1) == -1){
		fprintf(stderr,"Chamada semop() falhou, impossivel fechar o recurso Ocupa!");
		exit(1);
	}
#endif
	}
}

void produceLetter(int number, int count){
	int i, j;
	int tmp_index;

	tmp_index = *g_shm_prod_addr;

	//Produzindo as letras

	for(i = 0; i < number; i++){
		if(! (tmp_index + i > sizeof(g_letters_and_numbers)) ){

			g_shm_buffer_addr[tmp_index + i] = g_letters_and_numbers[tmp_index + i];
			fprintf(stderr,"%c", g_shm_buffer_addr[tmp_index + i]);
			usleep(1);

		}else{
			*g_shm_prod_addr = 0;
			fprintf(stderr,"\nFim do buffer(Produtor):");

			for(j = 0; j < TAM; j++){
		 		fprintf(stderr,"%c", *(g_shm_buffer_addr + j));
			}
			fprintf(stderr,"\n");
			return;
		}
	}
	fprintf(stderr," ");
	*g_shm_prod_addr = tmp_index + i;
	return;
}

void Consumer(){
	struct timeval tv;
	int number;

	usleep(1000);
	while(1){
	
		if(gettimeofday(&tv,NULL) == -1){
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}
		number = ((tv.tv_usec / 47) % 4) + 1;

	//P(Ocupa)
#ifdef PROTECT
	if(semop(g_sem_id[OCUPA], g_sem_op1, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso Ocupa!");
		exit(1);
	}
#endif
	//P(Emc)
#ifdef PROTECT
	if(semop(g_sem_id[EMC], g_sem_op1, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso EMC!");
		exit(1);
	}
#endif

		consumeLetter(number);

	//V(Emc)
#ifdef PROTECT
	if(semop(g_sem_id[EMC], g_sem_op2, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel abrir o recurso EMC!");
		exit(1);
	}
#endif

	//V(Livre)
#ifdef PROTECT
	if(semop(g_sem_id[LIVRE], g_sem_op2, 1) == -1){
		fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso Livre!");
		exit(1);
	}
#endif
	}
}

void consumeLetter(int number){
	int i, j;
	int tmp_index;

	tmp_index = *g_shm_consu_addr;

	//Produzindo as letras
	for(i = 0; i < number; i++){
		if(! (tmp_index + i > *g_shm_prod_addr) ){
			g_shm_buffer_addr[tmp_index + i] = '#';
			usleep(1);

		}else{
			if(*g_shm_consu_addr > sizeof(g_letters_and_numbers)){
				*g_shm_consu_addr = 0;

				fprintf(stderr,"\nFim do buffer(Consumidor):");
				for(j = 0; j < sizeof(g_letters_and_numbers); j++){
					fprintf(stderr,"%c", *(g_shm_buffer_addr + j));
				}
				return;

			}else {
				break;
			}
		}
	}
	*g_shm_consu_addr = tmp_index + i;
	return;
}

void initializeIPCS(){
	/*
	 * Criando os Semáforos
	 */	
	if( ( g_sem_id[EMP] = semget( SEM_KEY1, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if( ( g_sem_id[EMC] = semget( SEM_KEY2, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if( ( g_sem_id[LIVRE] = semget( SEM_KEY3, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if( ( g_sem_id[OCUPA] = semget( SEM_KEY4, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	

	//Estrutura de Fechamento dos semáforos
	for(int i = 0; i < 4; i++){
		g_sem_op1[i].sem_num   =  0;
		g_sem_op1[i].sem_op    = -1;
		g_sem_op1[i].sem_flg   =  0;
	}
	

	//Estrutura de Abertura dos semáforos
	for(int i = 0; i< 4; i++){
		g_sem_op2[i].sem_num =  0;
		g_sem_op2[i].sem_flg =  0;
		if( i == 2)
		g_sem_op2[i].sem_op  =  TAM;
		else
		 g_sem_op2[i].sem_op  =  1;
	}
	

	/*
	 * Inicializando os Semáforos
	 */	

	if( semop( g_sem_id[LIVRE], g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}
	
	if( semop( g_sem_id[EMC], g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	if( semop( g_sem_id[EMP], g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	/*
	 * Criando o segmento de memoria compartilhada do Buffer
	 */
	if( (g_shm_buffer_id = shmget( SHM_KEY1, sizeof(char)*TAM, IPC_CREAT | 0666 )) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	//Associando o segmento de Buffer
	if( (g_shm_buffer_addr = (char *)shmat(g_shm_buffer_id, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	//Criando o segmento de memoria compartilhada do Produtor
	if( (g_shm_prod_id = shmget( SHM_KEY2, sizeof(int), IPC_CREAT | 0666 )) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	//Associando o segmento de Produtor
	if( (g_shm_prod_addr = (int *)shmat(g_shm_prod_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	//Criando o segmento de memoria compartilhada do Consumidor
	if((g_shm_consu_id = shmget(SHM_KEY3, sizeof(int), IPC_CREAT| 0666)) == -1){
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	//Associando o segmento de Produtor
	if((g_shm_consu_addr = (int *)shmat(g_shm_consu_id, NULL, 0)) == (int *) -1){
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	*g_shm_prod_addr = 0;
	*g_shm_consu_addr =  0;
}

void removeIPCS(){
	int i;

	if( shmctl( g_shm_buffer_id, IPC_RMID,NULL) != 0 ) {
			fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada de buffer!\n");
			exit(1);
		}

	if( shmctl(g_shm_prod_id, IPC_RMID, NULL) != 0){
		fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada do produtor!\n");
		exit(1);
	}

	if(shmctl(g_shm_consu_id, IPC_RMID, NULL) != 0){
		fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada do consumidor!\n");
		exit(1);
	}

	//Removendo os semaforos
	
	if( semctl( g_sem_id[0], 0, IPC_RMID, 0) != 0 ) {
		fprintf(stderr,"Impossivel remover o conjunto de semaforos! %d\n", i);
		exit(1);
	}
}