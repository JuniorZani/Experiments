#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>			/* for printf() */
#include <stdlib.h>
#include <unistd.h>			/*For usleep*/
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>			/* for wait() */
#include <sys/ipc.h>		/* for msgget(), msgctl() */
#include <sys/msg.h>		/* for msgget(), msgctl() */


#define NO_OF_ITERATIONS	500

#define MICRO_PER_SECOND	1000000

#define MICRO_PER_SECOND	1000000

#define SENDER_DELAY_TIME	10

#define MESSAGE_MTYPE		1

typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
} data_t;

typedef struct {
	long mtype;
	char mtext[sizeof(data_t)];
} msgbuf_t;

/*Struct que será usada para armazenar os dados cálculados pelo filho receptor ao terceiro filho*/
typedef struct {
	float max;
	float min;
	float total;
	float average;
} data_results_t;

/*Struct que será usada como buffer de mensagem dos dados cálculados pelo filho receptor ao terceiro filho*/

typedef struct {
	long mtype;
	char mtext[sizeof(data_results_t)];
} resultbuf_t;

void Receiver(int first_queue_id, int second_queue_id, int multiplier);
void Sender(int queue_id, int multiplier);


int main(int argc, char * argv[]){

    int count;
    int queue_id[2];
	int multiplier;

	//Inicialização das variáveis
	count = atoi(argv[1]);
	queue_id[0] = atoi(argv[2]);
	queue_id[1] = atoi(argv[3]);
	multiplier = atoi(argv[4]);
	multiplier = multiplier * 512;

	//Declaração dos tipos criados
	resultbuf_t rmessage_buffer;
	data_results_t * results = (data_results_t * )(rmessage_buffer.mtext);

	//Filho 1
    if(count == 1) {
		//Receptor iniciado
		Receiver(queue_id[0], queue_id[1], multiplier);
		exit(0);

	//Filho 2
    } else if(count == 2) {
			//Emissor iniciado
			Sender(queue_id[0], multiplier);
			exit(0);

	//Filho 3
    } 	else if(count == 3){

			if( msgrcv(queue_id[1],(struct msgbuf *)&rmessage_buffer, sizeof(data_results_t), MESSAGE_MTYPE,0) == -1 ) {
				fprintf(stderr, "Impossivel receber mensagem!\n");
				exit(1);
			}
			printf("Valor máximo: %.8f\nValor mínimo: %.8f\nValor médio: %.8f\nValor total: %.8f\n",
			results->max, results->min, results->average, results->total);
			exit(0);
	}
    exit(0);
}


/*
 * Esta funcao executa o recebimento das mensagens
 */
void Receiver(int first_queue_id, int second_queue_id, int multiplier) {
	/*
	 * Variaveis locais
	 */
	int count;
	float delta;
	float max;
	float min;
	float total;
	struct timeval receive_time;

	/*
	 * Este eh o buffer para receber a mensagem
	 */
	msgbuf_t message_buffer;
	resultbuf_t results_message_buffer;
	results_message_buffer.mtype = MESSAGE_MTYPE;

 	//Este e o ponteiro para os dados no buffer. Note como e setado para apontar para o mtext no buffer
	data_t *data_ptr = (data_t *)(message_buffer.mtext);
	data_results_t *results = (data_results_t *)(results_message_buffer.mtext);

	/*
	 * Inicia o loop
	 */
	max = 0;
	
	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		/*
		 * Recebe qualquer mensagem do tipo MESSAGE_MTYPE
		 */	
		if( msgrcv(first_queue_id, (struct msgbuf *)&message_buffer, multiplier, MESSAGE_MTYPE,0) == -1 ) {
			fprintf(stderr, "Primeira Fila: Impossivel receber mensagem!\n");
			exit(1);
		}

		if(count == 0){
			min = data_ptr->send_time.tv_sec;
		}
		
		gettimeofday(&receive_time,NULL);

	/*
	 * Calcula a diferenca
	 */
		delta = receive_time.tv_sec  - data_ptr->send_time.tv_sec;
		delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec)/(float)MICRO_PER_SECOND;
		total += delta;

	/*
	 * Salva o tempo maximo e mínimo
	 */
		if( delta > max ) 
			max = delta;

		if(delta < min)
			min = delta;
	}

	//Atribuição dos valores calculados
	results->max = max;
	results->min = min;
	results->total = total;
	results->average = (total/NO_OF_ITERATIONS);

	//Envia a mensagem para o terceiro filho
	if(msgsnd(second_queue_id,(struct msgbuf *)&results_message_buffer, sizeof(data_results_t), 0) == -1 ){
		fprintf(stderr, "Segunda Fila: Impossivel enviar mensagem!\n");
		exit(1);
	}

    return;
}

/*
 * Esta funcao envia mensagens
 */
void Sender(int queue_id, int multiplier)
{
	/*
	 * Variaveis locais
	 */
	int count;
	struct timeval send_time;

	/*
	 * Este e o buffer para as mensagens enviadas
	 */
	msgbuf_t message_buffer;

	/*
	 * Este e o ponteiro para od dados no buffer.  Note
	 * como e setado para apontar para mtext no buffer
	 */
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	/*
	 * Inicia o loop
	 */
	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		/*
		 * Chama gettimeofday()
		 */
		gettimeofday(&send_time,NULL);

		/*
		 * Apronta os dados
		 */
		
		message_buffer.mtype = MESSAGE_MTYPE;
		data_ptr->msg_no = count;
		data_ptr->send_time = send_time;

		/*
		 * Envia a mensagem... usa a identificacao da fila, um ponteiro
		 * para o buffer, e o tamanho dos dados enviados
		 */
		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer, multiplier, 0) == -1 ) {
			fprintf(stderr, "Primeira Fila: Impossivel enviar mensagem!\n");
			exit(1);
		}
		//Dorme por um curto espaco de tempo 
		usleep(SENDER_DELAY_TIME);
	}
    return;
}