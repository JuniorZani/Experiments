#include <pthread.h>        /* for thread operations*/
#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>		    /* for printf() */
#include <stdlib.h>         /* for exit() */ 
#include <unistd.h>		    /* for fork() and usleep()*/
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>		    /* for wait() */
#include <sys/sem.h>        /* for semget(), semop(), semctl() */

#define SEM_CHAIRS_ID 0x1000
#define SEM_CUSTOMER_ID 0x1001

#define BARBER_TYPE 0
#define CUSTOMER_TYPE 1

#define BARBERS 2
#define CUSTOMERS 20
#define CHAIRS 7

#define MAX_SIZE 5115
#define MAX_RANGE 20

int sem_chair_id[CHAIRS];
int sem_customer_id;
int waiting;
int barberIndex, customerIndex;

pthread_t barbers[BARBERS];
pthread_t customers[CUSTOMERS];
pthread_mutex_t exclusive_access, barberInd, customerInd;

struct sembuf g_sem_lock[1];
struct sembuf g_sem_unlock[1];

//Estrutura que irá representar as cadeiras
typedef struct{
    int barberNum;
    int customerNum;
    char hair[MAX_SIZE];
    int cuttedHair[MAX_RANGE + 1];
    int size;
    struct timeval init_time;
}chair_t;

chair_t chair[CHAIRS];

void *barber(void *index);
void *customer(void *index);

void cutHair(char Array[], int sortedArray[], int size);
void apreciateHair(int chairIndex);

int insertIntoString(char Array[], char Number[], int index);
void insertIntoInt(char Arraychar[], char Number[], int Arrayint[], int size);

void bubbleSort(int Array[], int size);
void arrayCopy(char source[], char destination[], int size);
void swap(int array[], int index);

void ipcsCreat();
void ipcsRemove();

int main(){
    int tb, tc, count;

    ipcsCreat();

    for(count = 0; count < CUSTOMERS; count++){
        if(count < BARBERS){
            //Criação dos threads barbeiros
            printf("\nBarbeiro %d começou\n", count + 1);
            tb = pthread_create(&barbers[count], NULL, barber, (void *) (__intptr_t) count + 1);
            if(tb){
                fprintf(stderr, "\nImpossivel criar thread Barbeiro");
            }
        }
        //Criação dos threads Clientes
        printf("\nCliente %d começou\n", count + 1);
        tc = pthread_create(&customers[count], NULL, customer, (void*) (__intptr_t) count + 1);
        if(tc){
            fprintf(stderr, "\nImpossivel criar thread Cliente");
           }
        }

    for(count = 0; count < CUSTOMERS; count++){
        pthread_join(customers[count], NULL);
    }

    ipcsRemove();

    pthread_exit(NULL);
}

void *barber(void *ind){
    int index;
    int chairLocalIndex;
    
    index = (__intptr_t) ind;
    
    while(1){

        //P(CLI)
        if(semop(sem_customer_id, g_sem_lock, 1) == -1){
            fprintf(stderr,"Impossivel fechar semaforo cliente\n");
            exit(1);
        }

        //Waiting RC Lock
        pthread_mutex_lock(&exclusive_access);

        waiting--;

        //Waiting RC Unlock
        pthread_mutex_unlock(&exclusive_access);

        //barberInd RC Lock
        pthread_mutex_lock(&barberInd);

        barberIndex = (barberIndex + 1) % CHAIRS;
        chairLocalIndex = barberIndex;
        
        //barberInd RC Unlock
        pthread_mutex_unlock(&barberInd);

        chair[chairLocalIndex].barberNum = index;
        gettimeofday(&chair[chairLocalIndex].init_time, NULL);

        cutHair(chair[chairLocalIndex].hair, chair[chairLocalIndex].cuttedHair, chair[chairLocalIndex].size);
        
        //V(BARB)
        if(semop(sem_chair_id[chairLocalIndex], g_sem_unlock, 1) == -1){
            fprintf(stderr,"Impossivel abrir semaforo barber\n");
            exit(1);
        }
    }
}

void *customer(void *ind){

    int i, index, randNum;
    int hairIndex;
    int chairLocalIndex;
    char charAux[5];
    struct timeval time;

    index = (__intptr_t) ind;

    //Waiting RC Lock
    pthread_mutex_lock(&exclusive_access);

    if(waiting < CHAIRS){
        waiting++;
        
        //Waiting RC Unlock
        pthread_mutex_unlock(&exclusive_access);

        //customerInd RC Lock
        pthread_mutex_lock(&customerInd);

        customerIndex = (customerIndex + 1) % CHAIRS;
        chairLocalIndex = customerIndex;
        
        //customerInd RC Unlock
        pthread_mutex_unlock(&customerInd);

        gettimeofday(&time,NULL);
        chair[chairLocalIndex].customerNum = index;
        chair[chairLocalIndex].size = (time.tv_usec / 47) % MAX_RANGE + 2;

        hairIndex = 0;
        for(i = 0; i < chair[chairLocalIndex].size; i++){
            gettimeofday(&time,NULL);
            randNum = (time.tv_usec / 47) % MAX_RANGE + 2;
            sprintf(charAux, "%d" ,randNum);
            hairIndex = insertIntoString(chair[chairLocalIndex].hair, charAux, hairIndex);
        }
        chair[chairLocalIndex].hair[hairIndex] = '\0';

        //V(CLI)
        if(semop(sem_customer_id, g_sem_unlock, 1) == -1){
            fprintf(stderr,"Impossivel abrir semaforo cliente\n");
            exit(1);
        }

        //P(BARB)
        if(semop(sem_chair_id[chairLocalIndex], g_sem_lock, 1)){
            fprintf(stderr,"Impossivel abrir semaforo barbeiro\n");
            exit(1);
        }

        apreciateHair(chairLocalIndex);

    } else{
        fprintf(stderr, "Cliente %d não foi atendido", index);
        //Mutex Unlock
        pthread_mutex_unlock(&exclusive_access);
    }

    pthread_exit(NULL);
}

void cutHair(char Array[], int sortedArray[], int size){
    char auxiliarChar[5];
    insertIntoInt(Array, auxiliarChar, sortedArray, size);  //Transformando o vetor
    bubbleSort(sortedArray, size);
}

void apreciateHair(int chairIndex){
    float duration;
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    duration = endtime.tv_usec - chair[chairIndex].init_time.tv_usec;

    fprintf(stdout, "\nCliente %d cortou com Barbeiro %d\nTempo de espera: %.5f\nVetor antes da ordenção:\n%s",chair[chairIndex].customerNum, chair[chairIndex].barberNum, duration/1000, chair[chairIndex].hair);
    fprintf(stdout, "\nVetor depois da ordenção:\n");
    for(int i = 0; i < chair[chairIndex].size; i++){
        fprintf(stdout, "%d ", chair[chairIndex].cuttedHair[i]);
    }
    printf("\n");
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
            if (Array[j] > Array[j + 1])
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
    int i;

    //Inicialização das estruturas de controle do Semaforo
    g_sem_lock[0].sem_num = 0;
    g_sem_lock[0].sem_op = -1;
    g_sem_lock[0].sem_flg = 0;

    g_sem_unlock[0].sem_num = 0;
    g_sem_unlock[0].sem_op = 1;
    g_sem_unlock[0].sem_flg = 0;

    //Inicializando os indices de barbeiro e cliente
    barberIndex = CHAIRS - 1;
    customerIndex = CHAIRS - 1;

    for(i = 0; i < CHAIRS; i++){
        //Criação do conjunto de semaforos
        if( (sem_chair_id[i] = semget(SEM_CHAIRS_ID, 1, IPC_CREAT | 0666)) == -1){
            fprintf(stderr, "Impossivel criar semaforo\n");
            exit(1);
        }
    }
    
    //Criação do semáforo Cliente (CLI)
    if( (sem_customer_id = semget(SEM_CUSTOMER_ID, 1, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Impossivel criar semaforo\n");
        exit(1);
    }

    //inicialização da variável de "Mutual Exclusion"
    pthread_mutex_init(&exclusive_access, NULL);
    pthread_mutex_init(&barberInd, NULL);
    pthread_mutex_init(&customerInd, NULL);
}

void ipcsRemove(){
    //Remoção do cunjunto de semáforos referentes a cadeira
    if(semctl(sem_chair_id[BARBER_TYPE], IPC_RMID, 0) != 0){
        fprintf(stderr, "Impossivel remover conjunto de semaforos\n");
        exit(1);
    }
    //Remoção do conjunto de semáforos referente a cliente
    if(semctl(sem_customer_id, IPC_RMID, 0) != 0){
        fprintf(stderr, "Impossivel remover conjunto de semaforos\n");
        exit(1);
    }

    pthread_mutex_destroy(&exclusive_access);
    pthread_mutex_destroy(&barberInd);
    pthread_mutex_destroy(&customerInd);
}