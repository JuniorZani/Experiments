/***********************************************************************************
*
* Este programa n�o faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* embora tenha sido inspirado pelos demais experimentos lah existentes.
*
* Experimento # 4 na disciplina de Sistemas Operacionais da PUC-Campinas
* Originalmente programado por Florian Weizenegger
*							Data: 25/08/2003
* 
*       Proposito: O proposito deste programa e o de permitir ao aluno perceber
*       o que vem a ser um thread, de maneira tal que consiga distingui-lo de
*       um processo. Al�m disso, s�o usados os principais comandos para cria��o
*	e manipula��o de threads.
*	O problema dos produtores e consumidores sobre um buffer circular �
*	usado como assunto, permitindo que o aluno experimente duas implementa��es
*	diferentes para sua solu��o. Desta maneira, al�m dos threads propriamente
*	ditos, tambem locks e semaforos sao usados para garantir sincronizacao
*	de threads.
*
*************************************************************************************/

/*
 * Includes Necessarios 
 */

#include <pthread.h>			/* para poder manipular threads */
#include <stdlib.h>
#include <stdio.h>			/* para printf() */

/*
 * Constantes Necessarias 
 */
#define NUM_THREADS     10
#define SIZEOFBUFFER    50
#define NO_OF_ITERATIONS 100

/*
 * O tipo pthread_t permite a declara��o de uma vari�vel que recebe
 * um id quando o thread � criado. Posteriormente, esse id pode ser
 * usado em comandos de controle para threads.
 * Seguem dois vetores para ids, para um numero de threads igual a
 * constante NUM_THREADS
 */
pthread_t consumers[NUM_THREADS];
pthread_t producers[NUM_THREADS];

/*
 * Variaveis Necessarias 
 */
int buffer[SIZEOFBUFFER];		/* Este e um buffer circular	*/
int *start;				          /* apontara para a primeira posicao do buffer */
int *rp;				            /* eh o apontador para o proximo item do buffer a ser consumido */
int *wp;				            /* eh o apontador para o proximo item do buffer a ser produzido */
int cont_p = 0;             		/* eh um contador para controlar o numero de itens produzidos */
int cont_c = 0;         		/* eh um contador para controlar o numero de itens consumidos */


int myadd(int toAdd);
int myremove();
void *produce(void *threadid);
void *consume(void *threadid);

/*
 * A rotina consume e responsavel por chamar myremove para que seja
 * retorando um dos valores existentes no buffer NO_OF_ITERATIONS vezes 
 */

/*
 * Rotina Principal (que tambem e a thread principal, quando executada) 
 */
int main(int argc, char *argv[])
{
  int tp, tc;
  int i;

  start = &buffer[0];
  wp = start + SIZEOFBUFFER - 1;
  rp = start;
  
  for (i=0 ;i < NUM_THREADS; i++) {

    // tenta criar um thread consumidor
    tc = pthread_create(&consumers[i], NULL, consume, (void *)(__intptr_t)i+1);

    /* 
     * Pergunta 3: para que serve cada um dos argumentos usados com pthread_create?
     */

    if (tc) {
      printf("ERRO: impossivel criar um thread consumidor\n");
      exit(-1);
    }
    
    // tenta criar um thread produtor
    tp = pthread_create(&producers[i], NULL, produce, (void *)(__intptr_t)i+1);
    if (tp) {
      printf("ERRO: impossivel criar um thread produtor\n");
      exit(-1);
    }
  }
  printf("Terminando a thread main()\n");
  pthread_exit(NULL);

/* 
 * Pergunta 4: O que ocorre com as threads criadas, se ainda
 * estiverem sendo executadas e a thread que as criou termina
 * atrav�s de um pthread_exit()?
 */

/*
 * Pergunta 5: Idem � quest�o anterior, se o termino se da atraves
 * de um exit()?
 */
}


/*
 * Rotina para produzir um item toAdd no buffer 
 */
int myadd(int toAdd) {
    wp++;
    //verificacao se wp chegou a ultima posicao do buffer
    if (wp == (start + SIZEOFBUFFER)) {
      wp = start;
    }
    *wp = toAdd;
    return 1;
}

/*
 * Rotina para consumir um item do buffer e coloca-lo em retValue 
 */
int myremove() {
  int retValue;

  //verificacao se o buffer nao esta vazio
  if(cont_p != 0){
    retValue = *rp;
    rp++;

    //Verificacao se rp chegou a ultima posicao do buffer
    if (rp == (start + SIZEOFBUFFER)) {
        rp = start;				/* realiza a circularidade no buffer */
      }
    return retValue;
  }
  return 0;
}

/*
 * A rotina produce e responsavel por chamar myadd para que seja 
 * colocado o valor 10 em uma posicao do buffer NO_OF_ITERATIONS vezes
 */
void *produce(void *id){
  
  int ret, sum, threadid;

  sum = 0;
  threadid = (__intptr_t)id;
    
  printf("Produtor #%d iniciou...\n", threadid);

  while (cont_p < NO_OF_ITERATIONS) {
    ret = myadd(10);
    if (ret) {
    /* 
     * Pergunta 1: porque ret n�o est� sendo comparado a algum valor?
     * Pergunta 2: porque nao ha necessidade de um cast?
     */
      cont_p++;
      sum += 10;
    }
  }
  printf("Soma produzida pelo Produtor #%d : %d\n", threadid, sum);
  pthread_exit(NULL);

}

/*
 * A rotina consume e responsavel por chamar myremove para que seja
 * retorando um dos valores existentes no buffer NO_OF_ITERATIONS vezes 
 */
void *consume(void *id){
  int ret, sum, threadid;

  sum = 0;
  threadid = (__intptr_t)id;

  printf("Consumidor #%d iniciou...\n", threadid);

  while (cont_c < NO_OF_ITERATIONS) {
    ret = myremove();
    if (ret) {
      cont_c++;
      sum += ret;
    }
  }
  printf("Soma do que foi consumido pelo Consumidor #%d : %d\n", threadid, sum);
  pthread_exit(NULL);
}
