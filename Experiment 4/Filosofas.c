#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //Para usleep
#include <sys/types.h>
#include <sys/sem.h>    //Para semget(), semop(), semctl()


#define PENSANDO 0
#define COM_FOME 1
#define COMENDO 2
#define NUM_OF_PHILOSOPHERS 5
#define ESQUERDA (filoID + (NUM_OF_PHILOSOPHERS - 1))%NUM_OF_PHILOSOPHERS
#define DIREITA (filoID + (NUM_OF_PHILOSOPHERS + 1))%NUM_OF_PHILOSOPHERS
#define SEM_KEY 0x1234

pthread_t ID_Filosofa[NUM_OF_PHILOSOPHERS];
pthread_mutex_t mutex;

struct sembuf g_sem_ps[1];
struct sembuf g_sem_nps[1];


int estado[NUM_OF_PHILOSOPHERS];
int sem_id[NUM_OF_PHILOSOPHERS];

void *Filosofa(void *threadID);
void PegaGarfo(int filoID);
void ColocaGarfo(int filoID);
void Testa(int filoID);
void ipcsCreat(int i);
void ipcsRemove(int i);
void showStates(int filoID);

int main(int argc, char *argv[]){
    int i;

    //Procedimento que cria e inicializa os IPC's
    ipcsCreat(i);

    //Thread Main espera até o fim das Filosofas
    for(i = 0; i < NUM_OF_PHILOSOPHERS; i++){
        pthread_join(ID_Filosofa[i], NULL);
    }

    ipcsRemove(i);
    //Finalização do thread Main
    pthread_exit(NULL);

}

void *Filosofa(void * i){
    int ID = (__intptr_t) i;
    int come = 0;

    while(come < 365){
        usleep(25);
        PegaGarfo(ID);
        come++;
        ColocaGarfo(ID);
    }

    printf("Filosofa %d terminou!\n", ID + 1);
    pthread_exit(NULL);
}

void PegaGarfo(int filoID){
    int i;
    int em;

    //P(E_M)
    if((em = pthread_mutex_lock(&mutex)) != 0){
        fprintf(stderr, "Impossível fechar o recurso\n");
        exit(-1);
    }
   
    showStates(filoID);

    estado[filoID] = COM_FOME;
    
    Testa(filoID);
    
    //V(E_M)
    if((em = pthread_mutex_unlock(&mutex)) != 0){
        fprintf(stderr, "Impossível abrir o recurso\n");
        exit(-1);
    }

    //P(PodeSentar[filoID]);
    if((semop(sem_id[filoID], g_sem_nps, 1)) == -1){
        fprintf(stderr, "Impossivel fechar o semaforo\n");
        exit(-1);
    }
    return;
}

void ColocaGarfo(int filoID){
    int em;
    
    //P(E_M)
    if((em = pthread_mutex_lock(&mutex)) != 0){
        fprintf(stderr, "Impossível fechar o recurso\n");
        exit(-1);
    }

    estado[filoID] = PENSANDO;

    Testa(ESQUERDA);
    Testa(DIREITA);

    //V(E_M)
    if((em = pthread_mutex_unlock(&mutex)) != 0){
        fprintf(stderr, "Impossível abrir o recurso\n");
        exit(-1);
    }
    return;
}

void Testa(int filoID){

    if((estado[filoID] == COM_FOME) && (estado[ESQUERDA] != COMENDO) && (estado[DIREITA] != COMENDO)){
        estado[filoID] = COMENDO;

        //V(PODE_SENTAR)
        if((semop(sem_id[filoID], g_sem_ps, 1)) == -1){
            fprintf(stderr, "Impossivel abrir o semaforo, filo: %d\n", filoID);
            exit(-1);
        }
    }
 	return;
}

void ipcsCreat(int i){
    int tf;

    //Estrutura de Abertura dos Semáforos
    g_sem_ps[0].sem_num = 0;
    g_sem_ps[0].sem_op = 1;
    g_sem_ps[0].sem_flg = 0;

    //Estrutura de Fechamento dos Semáforos
    g_sem_nps[0].sem_num = 0;
    g_sem_nps[0].sem_op = -1;
    g_sem_nps[0].sem_flg = 0;

    pthread_mutex_init(&mutex, NULL);
    
    //Criando os semáforos
    for(i = 0; i < NUM_OF_PHILOSOPHERS; i++){
        if((sem_id[i] = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1){
                fprintf(stderr, "Impossivel criar semaforos\n");
                exit(-1);
        }
    }

    //Tentativa de Criação das Threads Filosofas
    for(i = 0; i < NUM_OF_PHILOSOPHERS; i++){
        tf = pthread_create(&ID_Filosofa[i], NULL, Filosofa, (void *)(__intptr_t)i);
        if(tf){
            fprintf(stderr, "Impossivel criar o thread #%d\n", i);
            exit(-1);
        }
        estado[i] = 0;
    }
    return;
}

void ipcsRemove(int i){
    
    //Remoção dos semáforos
    if((semctl(sem_id[0], 1, IPC_RMID)) == -1){
        fprintf(stderr, "Impossivel remover o semaforos\n");
        exit(-1);
    }

    //Destruição do Mutex criado
    pthread_mutex_destroy(&mutex);
    return;
}

void showStates(int filoID){
    int i;
    printf("Filosofa %d tentou comer...\nEstados:\t", filoID + 1);
    for(i = 0; i < NUM_OF_PHILOSOPHERS; i++){
        printf(" %d", estado[i]);
    }
    printf("\n\n");
}