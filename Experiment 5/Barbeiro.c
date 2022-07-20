#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>		    /* for printf() */
#include <stdlib.h>         /* for exit() */ 
#include <unistd.h>		    /* for fork() and usleep()*/
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>		    /* for wait() */
#include <sys/ipc.h>	    /* for msgget(), msgctl() */
#include <sys/msg.h>	    /* for msgget(), msgctl() */
#include <sys/sem.h>        /* for semget(), semop(), semctl() */
#include <sys/shm.h>        /* for shmget(), shmat(), shmctl() */

#define SEM_KEY             0x1000
#define SHM_KEY             0x1001
#define MESSAGE_QUEUE_ID	3102

#define BARBER_TYPE 0
#define CUSTOMER_TYPE 1

#define BARBERS 2
#define CUSTOMERS 20
#define CHAIRS 5

int sem_ID, shm_ID, queue_ID;
int *waiting;

typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
}data_t;

typedef struct {
    long mtype;
    char mtext[sizeof(data_t)];
}msg_buffer;

//Estrutura que será usada para controle do semaforo
struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];

void barber(int index);
void customer(int index);
void cutHair();
void apreciateHair();
void ipcsCreat();       //Done
void ipcsRemove();      //Done

int main(){
    int rtn, count;
    int pid[BARBERS + CUSTOMERS];

    rtn = 1;
    for(count = 0; count < BARBERS + CUSTOMERS; count++){
        if(rtn != 0)
            pid[count] = rtn = fork();
        else
            break;
    }

    if(rtn == 0){
        if(count < BARBERS){
        printf("Barbeiro %d começou!\n", count);
        barber(count);
    }else
        if(count < CUSTOMERS){
            printf("Consumidor %d começou!\n", count);
            customer(count);
        }
    }else{
        for(count = 0; count < BARBERS + CUSTOMERS)
            wait(NULL);
        ipcsRemove();
    }

    return 0;
}

void barber(int index){
    while(true){
        //Recebe  a mensagem do cliente ou é bloqueado TODO

        //P(ac_ex)
        if(semop(sem_ID, g_sem_op1, 1)){
            fprintf(stderr,"Impossivel fechar semaforo");
            exit(1);
        }

        *waiting--;

        //V(ac_ex)
        if(semop(sem_ID, g_sem_op2, 1)){
            fprintf(stderr,"Impossivel abrir semaforo");
            exit(1);
        }

        cutHair();
        //Envia a mensagem ao cliente TODO
        
    }
}

void customer(int index){
    //P(ac_ex)
    if(semop(sem_ID, g_sem_op1, 1)){
        fprintf(stderr,"Impossivel fechar semaforo");
        exit(1);
    }

    if(*waiting < CHAIRS){
        *waiting++;
        //Envia mensagem ao barbeiro TODO

        //V(ac_ex)
        if(semop(sem_ID, g_sem_op2, 1)){
            fprintf(stderr,"Impossivel abrir semaforo");
            exit(1);
        }

        //Recebe a mensagem do barbeiro ou é bloqueado TODO
        apreciateHair();

    } else
        //V(ac_ex)
        if(semop(sem_ID, g_sem_op2, 1)){
        fprintf(stderr,"Impossivel abrir semaforo");
        exit(1);
    }

}

void ipcsCreat(){
    key_t key = MESSAGE_QUEUE_ID;

    //Inicialização das estruturas de controle do Semaforo
    g_sem_op1[1].sem_num = 0;
    g_sem_op1[1].sem_op = -1;
    g_sem_op1[1].sem_flg = 0;

    g_sem_op2[1].sem_num = 0;
    g_sem_op2[1].sem_op = 1;
    g_sem_op2[1].sem_flg = 0;

    //Criação do semaforo
    if( (sem_ID = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar semaforo");
        exit(1);
    }

    //Inicialização do semaforo como ABERTO
    if(semop(sem_ID, g_sem_op2, 1)){
        fprintf(stderr,"Impossivel inicializar semaforos");
        exit(1);
    }

    //Criação da fila de mensagem
    if((queue_ID = msgget(key, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar fila de mensagens");
        exit(1);
    }

    //Criação da memória compartilhada
    if((shm_ID = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar memoria compartilhada");
        exit(1);
    }

    //Associação e inicialização da memoria compartilhada
    if((waiting = (int*) shmat(shm_ID, NULL, 0)) == (int*)-1){
        fprintf(stderr, "Impossivel associar segmento de memoria compartilhada");
        exit(1);
    }
    *waiting = 0;
}

void ipcsRemove(){
    //Remoção do semaforo
    if(semctl(sem_ID, IPC_RMID, 0) != 0){
        fprintf(stderr, "Impossivel remover conjunto de semaforos");
        exit(1);
    }
    //Remoção da fila de mensagens
    if(msgctl(queue_ID, IPC_RMID, NULL) != 0){
        fprintf(stderr, "Impossivel remover a fila de mensagens");
        exit(1);
    }
    //Remoção da memória compartilhada
    if(shmctl(shm_ID, IPC_RMID, NULL) != 0){
        fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada");
        exit(1);
    }
}

void cutHair(){
    printf("Cortou, e ordenou o vetor\n");
}

void apreciateHair(){
    printf("Apreciou, e exibiu o vetor\n");
}