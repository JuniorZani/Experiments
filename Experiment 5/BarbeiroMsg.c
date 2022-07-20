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
#include <string.h>
#include <signal.h>

#define SEM_KEY             0x1000
#define SHM_KEY             0x1001
#define MESSAGE_QUEUE_ID	3102
#define MAX_SIZE            5115
#define MAX_RANGE           50

#define CUSTOMER_TYPE 1

#define BARBERS 2
#define CUSTOMERS 20
#define CHAIRS 7

int sem_ID, shm_ID, queue_ID;
int *waiting;

//Estrutura de dados da mensagem do cliente
typedef struct {
	int size;
    int customer_num;
    char array[MAX_SIZE];
}customer_data_t;

//Estrutura de dados da mensagem do barbeiro
typedef struct{
    int size;
    int customer_num;
    int barber_num;
    char array[MAX_SIZE];
    int sortedArray[MAX_RANGE + 1];
    struct timeval time;
}barber_data_t;

//Estrutura de buffer da mensagem do cliente
typedef struct {
    long mtype;
    char mtext[sizeof(customer_data_t)];
}msg_customer_buffer;

//Estrutura de buffer da mensagem do barbeiro
typedef struct {
    long mtype;
    char mtext[sizeof(barber_data_t)];
}msg_barber_buffer;

//Estrutura que será usada para controle do semaforo
struct sembuf g_sem_lock[1];
struct sembuf g_sem_unlock[1];

void barber(int index);
void customer(int index);

void cutHair(char Array[], int sortedArray[], int size);
void apreciateHair(int barberNum, int customerNum, char array[], int sortedArray[], int size, struct timeval begintime);

int insertIntoString(char Array[], char Number[], int index);
void insertIntoInt(char Arraychar[], char Number[], int Arrayint[], int size);

void bubbleSort(int Array[], int size);
void arrayCopy(char source[], char destination[], int size);
void swap(int array[], int index);

void ipcsCreat();
void ipcsRemove(int pids[]);

int main(){
    int rtn, count;
    int pid[BARBERS + CUSTOMERS];

    ipcsCreat();

    rtn = 1;
    for(count = 0; count < BARBERS + CUSTOMERS; count++){
        if(rtn != 0)
            pid[count] = rtn = fork();
        else
            break;
    }
    //Se é filho
    if(rtn == 0){
        if(count <= BARBERS){
            printf("\nBarbeiro %d começou!\n", count);
            barber(count);
        }
        if(count > BARBERS){
            printf("\nCliente %d começou!\n", count - BARBERS);
            customer(count);
            exit(0);
        }
    //Se é pai
    }else{
        fprintf(stderr, "\nPai Aguardando");
        for(count = 0; count < CUSTOMERS; count++){
            wait(NULL);
        }
        ipcsRemove(pid);
    }
    exit(0);
}

void barber(int index){

    msg_barber_buffer barbMsg;
    barber_data_t *barb_data_ptr = (barber_data_t*) (barbMsg.mtext);
    
    msg_customer_buffer cstmrMsg;
    customer_data_t *customer_data_ptr = (customer_data_t*) (cstmrMsg.mtext);
    
    while(1){
        //Recebe a mensagem do cliente ou é bloqueado
        if(msgrcv(queue_ID, (struct msg_customer_buffer*) &cstmrMsg, sizeof(customer_data_t), CUSTOMER_TYPE, 0) == -1){
            fprintf(stderr, "Impossivel receber mensagem do cliente");
            exit(1);
        }
        
        //P(ac_ex)
        if(semop(sem_ID, g_sem_lock, 1)){
            fprintf(stderr,"\nImpossivel fechar semaforo no barbeiro\n");
            exit(1);
        }

        (*waiting)--;

        //V(ac_ex)
        if(semop(sem_ID, g_sem_unlock, 1)){
            fprintf(stderr,"\nImpossivel abrir semaforo no barbeiro\n");
            exit(1);
        }

        gettimeofday(&barb_data_ptr->time, NULL);

        barb_data_ptr->barber_num = index;
        barb_data_ptr->customer_num = customer_data_ptr->customer_num;
        barb_data_ptr->size = customer_data_ptr->size;

        strcpy(barb_data_ptr->array, customer_data_ptr->array);
        cutHair(barb_data_ptr->array, barb_data_ptr->sortedArray, barb_data_ptr->size);

        barbMsg.mtype = (long) customer_data_ptr->customer_num;

        //Envia a mensagem ao cliente
        if(msgsnd(queue_ID, (struct msg_barber_buffer*) &barbMsg, sizeof(barber_data_t), 0) == -1){
            fprintf(stderr, "\nImpossivel enviar mensagem ao cliente");
            exit(1);
        }
    }
}

void customer(int index){
   int i, randNum, arrayIndex;
   char charAux[5];
   struct timeval time;

    msg_barber_buffer barbMsg;
    barber_data_t *barb_data_ptr = (barber_data_t*) (barbMsg.mtext);
    
    msg_customer_buffer cstmrMsg;
    customer_data_t *customer_data_ptr = (customer_data_t*) (cstmrMsg.mtext);

    cstmrMsg.mtype = CUSTOMER_TYPE;
    customer_data_ptr->customer_num = index;

    //P(ac_ex)
    if(semop(sem_ID, g_sem_lock, 1)){
        fprintf(stderr,"\nImpossivel fechar semaforo no cliente\n");
        exit(1);
    }

    if((*waiting) < CHAIRS){
        (*waiting)++;

        //Gera o tamanho da string dentre 2 a 1023 e a preenche com valores randomicos de mesmo range
        gettimeofday(&time,NULL);
        customer_data_ptr->size = (time.tv_usec / 47) % MAX_RANGE + 2;

        arrayIndex = 0;
        for(i = 0; i < customer_data_ptr->size; i++){
            gettimeofday(&time,NULL);
            randNum = (time.tv_usec / 47) % MAX_RANGE + 2;
            sprintf(charAux, "%d" ,randNum);
            arrayIndex = insertIntoString(customer_data_ptr->array, charAux, arrayIndex);
            usleep(2);
        }
        customer_data_ptr->array[arrayIndex] = '\0';

        //Envia mensagem ao barbeiro com os dados atribuidos
        if(msgsnd(queue_ID, (struct msg_customer_buffer*) &cstmrMsg, sizeof(customer_data_t), 0) == -1){
            fprintf(stderr, "\nImpossivel enviar a mensagem ao barbeiro");
            exit(1);
        }

        //V(ac_ex)
        if(semop(sem_ID, g_sem_unlock, 1)){
            fprintf(stderr,"\nImpossivel abrir semaforo\n");
            exit(1);
        }

        //Recebe a mensagem do barbeiro com o array ordenado e as informações necessárias para a impressão

        if(msgrcv(queue_ID, (struct msg_barber_buffer*) &barbMsg, sizeof(barber_data_t), index, 0) == -1){
            fprintf(stderr, "Impossivel receber mensagem do barbeiro");
            exit(1);
        }

        apreciateHair(barb_data_ptr->barber_num, barb_data_ptr->customer_num,
         barb_data_ptr->array, barb_data_ptr->sortedArray, customer_data_ptr->size, barb_data_ptr->time);

    } else{
        fprintf(stderr,"Cliente %d não foi atendido", index);
        //V(ac_ex)
        if(semop(sem_ID, g_sem_unlock, 1)){
            fprintf(stderr,"Impossivel abrir semaforo customer\n");
            exit(1);
        }
    }
}
//Ordena o vetor gerado pelo cliente
void cutHair(char Array[], int sortedArray[], int size){
    char auxiliarChar[5];
    insertIntoInt(Array, auxiliarChar, sortedArray, size);  //Transformando o vetor
    bubbleSort(sortedArray, size);
}

void apreciateHair(int barberNum, int customerNum, char array[], int sortedArray[], int size, struct timeval begintime){
    float duration;
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    duration = endtime.tv_usec - begintime.tv_usec;

    fprintf(stdout, "\nCliente %d cortou com Barbeiro %d\nTempo de espera: %.5f\nVetor antes da ordenção:\nCliente %d: %s",customerNum - BARBERS, barberNum, duration/1000, customerNum - BARBERS,array);
    fprintf(stdout, "\nVetor depois da ordenção:\nCliente %d: ", customerNum - BARBERS);
    for(int i = 0; i < size; i++){
        fprintf(stdout, "%d ", sortedArray[i]);
    }
    printf("\n");
}

void arrayCopy(char source[], char destination[], int size){
    for(int i = 0; i < size; i++){
        destination[i] = source[i];
    }
}

int insertIntoString(char Array[], char Number[], int index){
    int i, count = 0;
    count = 0;

    for(i = index; i < index + 5; i++){
        if(Number[count] == '\0'){
            Array[i] = ' ';
            break;
        }else{
            Array[i] = Number[count];
            count++;
        }
    }
    return index + count + 1;
}
//Gera um vetor de inteiros a partir do string
void insertIntoInt(char Arraychar[], char Number[], int Arrayint[], int size){
    
    int intIndex, charIndex;
    intIndex = charIndex = 0;

    for(intIndex = 0; intIndex < size; intIndex++){
        for(int j = 0; j < 5; j++){
            if(Arraychar[charIndex] != ' ' && Arraychar[charIndex] != '\0')
                Number[j] = Arraychar[charIndex];
            else{
                charIndex++;
                break;
            }  
            charIndex++;
        }
        Arrayint[intIndex] = atoi(Number);
    }
}

void bubbleSort(int Array[], int size){
    int i, j;
    for (i = 0; i < size - 1; i++){
        for (j = 0; j < size - i - 1; j++){
            if (Array[j] < Array[j + 1])
                swap(Array, j);
        }
    }
}

void swap(int array[], int index){
    int aux;
    aux = array[index];
    array[index] = array[index + 1];
    array[index + 1] = aux;
}

void ipcsCreat(){
    key_t key = MESSAGE_QUEUE_ID;

    //Inicialização das estruturas de controle do Semaforo
    g_sem_lock[0].sem_num = 0;
    g_sem_lock[0].sem_op = -1;
    g_sem_lock[0].sem_flg = 0;

    g_sem_unlock[0].sem_num = 0;
    g_sem_unlock[0].sem_op = 1;
    g_sem_unlock[0].sem_flg = 0;

    //Criação do semaforo
    if( (sem_ID = semget(SEM_KEY, 1, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar semaforo\n");
        exit(1);
    }

    //Inicialização do semaforo como ABERTO
    if(semop(sem_ID, g_sem_unlock, 1)){
        fprintf(stderr,"Impossivel inicializar semaforos\n");
        exit(1);
    }

    //Criação da fila de mensagem
    if((queue_ID = msgget(key, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar fila de mensagens\n");
        exit(1);
    }

    //Criação da memória compartilhada
    if((shm_ID = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar memoria compartilhada\n");
        exit(1);
    }

    //Associação e inicialização da memoria compartilhada
    if((waiting = (int*) shmat(shm_ID, NULL, 0)) == (int*)-1){
        fprintf(stderr, "Impossivel associar segmento de memoria compartilhada\n");
        exit(1);
    }
    (*waiting) = 0;
}

void ipcsRemove(int pids[]){
    
    //Finalizando os Filhos
    for(int i = 0; i < BARBERS + CUSTOMERS; i++){
        kill(pids[i], SIGKILL);
    }

    //Remoção do semaforo
    if(semctl(sem_ID, IPC_RMID, 0) != 0){
        fprintf(stderr, "Impossivel remover conjunto de semaforos\n");
        exit(1);
    }
    //Remoção da fila de mensagens
    if(msgctl(queue_ID, IPC_RMID, NULL) != 0){
        fprintf(stderr, "Impossivel remover a fila de mensagens\n");
        exit(1);
    }
    //Remoção da memória compartilhada
    if(shmctl(shm_ID, IPC_RMID, NULL) != 0){
        fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada\n");
        exit(1);
    }
}