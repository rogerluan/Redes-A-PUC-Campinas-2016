#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <unistd.h> // for close
#include <sys/wait.h> //para o wait
#include <pthread.h>

typedef struct  {
    char nome[11];
    char msg[51];
} mensagemDBS;

typedef struct {
    struct sockaddr_in cli;
    //pthread_t tid;
    int s;  /* Socket conectado ao cliente        */
} paramCliente;


/*
 * Constantes Necessarias 
 */

#define TAM_BUF 62*5+3
#define protocol_i 0
#define MAXMSGS 5


/*Variaveis globais*/

pthread_t thread_id;
pthread_mutex_t mutex;

int mensagensCadastradas = 0; //contador de mensagens cadastradas
mensagemDBS mensagensDB[MAXMSGS];




	
enum codigos_da_vinci { cadastrar = 1, ler, apagar, ack, erro, invalido, querSair,vazio };
int sair = 0;


void sendResp(char s[TAM_BUF],int ns, int resp ) {
    s[protocol_i] = resp;
    if (send(ns, s,1, 0) < 0)
        perror("Send()");
}   

int getsPutBuff(char buf[TAM_BUF], int j,int size) {
    char s[size+1];
    int i;
    fgets(s,size,stdin);
    for (i=0; s[i] ; i++) {
        if (s[i] == '\n') break;
        buf[j++] = s[i];
    }
    buf[j++] = '\0';
    return j;
}

int putBuff(char buf[TAM_BUF], int j, char s[]) {
    int i = 0;
    for (; s[i];i++) {
        buf[j++] = s[i];
    }
    buf[j++] = '\0';
    return j;
}

int getNextString(char buf[TAM_BUF], int j) {
    while (buf[j++] != '\0');
    if (buf[j] == '#') return -1;
    return j;
}

void *trata_cliente(void *client_connection){

    /*Variaveis exclusivas da thread*/
    socklen_t ns;
    int l;
    char sendbuf[TAM_BUF], recvbuf[TAM_BUF];  
    int i=0, j=0, tid;

    int lidos, conectado = 1;

    printf("ns...");
    paramCliente* par = (paramCliente*) (client_connection);
    ns = par->s;
    //tid = par->tid;
    tid = pthread_self();
    struct sockaddr_in client = par->cli;
    printf("thr[%u]: Cliente se conectou com  %d\n",(unsigned) tid,ns);
    while (conectado) {
        printf("thr[%u]:  Aguardando mensagem do cliente\n", (unsigned)tid);
        if ( (lidos = recv(ns, recvbuf, sizeof(recvbuf), 0)) == -1)  {
            perror("Recv()");
            continue;
        }
        switch ( recvbuf[protocol_i]) {
            case cadastrar:                
                printf("thr[%u]: O cliente na porta %d deseja cadastrar uma mensagem\n",(unsigned) tid,ntohs(client.sin_port));
                pthread_mutex_lock(&mutex);
                if (mensagensCadastradas >= MAXMSGS) {
                    pthread_mutex_unlock(&mutex);
                    printf("thr[%u]: ERRO! Limite de mensagens cadastradas atingido. Enviando mensagem de erro ao cliente da porta %d\n",(unsigned)tid,ntohs(client.sin_port));
                    sendResp(sendbuf,ns,erro);                     
                    break;
                }
                printf("thr[%u]: Cadastrando a mensagem\n",(unsigned)tid);
                strcpy(mensagensDB[mensagensCadastradas].nome,recvbuf+1);
                l = getNextString(recvbuf,1);
                strcpy(mensagensDB[mensagensCadastradas].msg,recvbuf+l);
                printf("thr[%u]: Cadastrado: Nome: %s, Mensagem: %s. Enviando Confirmacao ao cliente da porta %d\n",(unsigned)tid,mensagensDB[mensagensCadastradas].nome,mensagensDB[mensagensCadastradas].msg,ntohs(client.sin_port));
                sendResp(sendbuf,ns,ack);
                mensagensCadastradas++;
                pthread_mutex_unlock(&mutex);
                break;
            case apagar:
                l = 1;
                int apagadas = 0;
                printf("thr[%u]: Cliente da porta %d deseja apagar mensagens de %s\n",(unsigned)tid,ntohs(client.sin_port),recvbuf+1);
                pthread_mutex_lock(&mutex);
                for (i =0; i < mensagensCadastradas;i++) {
                    if (strcmp(recvbuf+1,mensagensDB[i].nome) == 0) { 
                        l = putBuff(sendbuf,l,mensagensDB[i].nome); 
                        l = putBuff(sendbuf,l,mensagensDB[i].msg);
                        printf("thr[%u]: apagando mensagem de %s : %s...como pedido pelo cliete da porta %d\n",(unsigned) tid,mensagensDB[i].nome,mensagensDB[i].msg,ntohs(client.sin_port));                
                        for (j = i+1; j<mensagensCadastradas;j++)   //'puxa' as mensagens subsequentes, deletando a atual
                            mensagensDB[j-1] = mensagensDB[j];      //não é eficiente, mas n vou criar uma lista ligada '-'
                        mensagensCadastradas--;
                        i--;                      
                        apagadas++;
                    }
                }
                pthread_mutex_unlock(&mutex);
                if (apagadas == 0) {
                    sendResp(sendbuf,ns,vazio);
                } else {
                    sendbuf[protocol_i] = ack;
                    sendbuf[l] = '#'; //sinaliza sinal da lista de mensagens
                    if (send(ns, sendbuf,l+1, 0) < 0)
                        perror("Send()");
                }               
                break;
            case querSair:
                printf("thr[%u]: Cliente da porta %d deseja encerrar a conexao.\n",(unsigned) tid,ntohs(client.sin_port));
                conectado = 0;
                break;
            case ler:
                printf("thr[%u]: Cliente da porta %d deseja ler as mensagens. Listando as mensagens cadastradas\n",(unsigned) tid,ntohs(client.sin_port));
                pthread_mutex_lock(&mutex);
                if (mensagensCadastradas == 0) {
                    pthread_mutex_unlock(&mutex);
                    printf("thr[%u]: Nenhuma mensagem cadastrada. Enviando informacao ao cliente da porta %d.\n",(unsigned) tid,ntohs(client.sin_port));
                    sendResp(sendbuf,ns,vazio);
                    break;
                }
                l = 1;
                for (i =0; i < mensagensCadastradas;i++) {
                    l = putBuff(sendbuf,l,mensagensDB[i].nome); 
                    l = putBuff(sendbuf,l,mensagensDB[i].msg);
                    printf("thr[%u]: Adicionando mensagem: Autor: %s Conteudo: %s, ao pacote para enviar ao cliente da porta %d\n",(unsigned) tid,mensagensDB[i].nome,mensagensDB[i].msg,ntohs(client.sin_port));                
                }
                pthread_mutex_unlock(&mutex);
                sendbuf[protocol_i] = ack;
                sendbuf[l] = '#'; //sinaliza final da lista de mensagens
                if (send(ns, sendbuf,l+1, 0) < 0)
                        perror("Send()");
                break;
            default:
                printf("thr[%u]: Comando de código %d desconhecido. Enviando resposta ao cliente da porta %d\n",(unsigned) tid,recvbuf[protocol_i],ntohs(client.sin_port));
                sendResp(sendbuf,ns,invalido); 
                break;    
        }
    }
    pthread_exit(0);
}



int main(int argc,char *argv[])
{
    unsigned short port;                   
    struct sockaddr_in client; 
    struct sockaddr_in server; 
    socklen_t s;                     /* Socket para aceitar conexões       */                    
    socklen_t namelen;
    //signal(SIGCHLD,receive_child_signal);

    pthread_mutex_init(&mutex, NULL);


    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2) {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
        exit(2);
    }

   /*
    * Define a qual endereço IP e porta o servidor estará ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endereços IP
    */
    server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)  {
       perror("Bind()");
       exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    printf("Bind ok. Preparando listen\n");
    if (listen(s, 1) != 0) {
        perror("Listen()");
        exit(4);
    }
    printf("listen ok. Chamando accept\n");
    /*
     * Aceita uma conexão e cria um novo socket através do qual
     * ocorrerá a comunicação com o cliente.
     */

    namelen = sizeof(client);

    while (1) {        
        printf("Servidor pronto e aguardando novo cliente\n");
        paramCliente* p = (paramCliente*) malloc(sizeof(paramCliente));
        if ((p->s = accept(s, (struct sockaddr *)&p->cli, &namelen)) == -1) {
            perror("Accept()");
            exit(5);
        }

        printf("\nCriando Thread de atendimento para o cliente na porta %d, handler %d\n",ntohs(p->cli.sin_port),p->s);
    	
        pthread_create(&thread_id, NULL, trata_cliente, (void *)p);//pthread_create();
	pthread_detach(thread_id);
    		
    	if ((int *)thread_id > 0)  {
    		printf("Thread filha criada: %u\n", (unsigned) thread_id);
		pthread_detach(thread_id);
    	} else	{
    	    perror("Thread creation!!");
    	    exit(7);
    	}
	}
    printf("Servidor terminou com sucesso.\n");
    exit(0);
}
