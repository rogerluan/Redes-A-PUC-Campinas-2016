//
//  WhatsAp2p Server.c
//  WhatsAp2p Server
//
//  Created by Roger Luan on 6/3/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

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
#include <sys/wait.h> // wait
#include <pthread.h>


#pragma mark - Global Variables

#define TAM_BUF 400 //to-do:
#define PHONE_DIGITS 11
#define protocol_index 0
pthread_t thread_id;
pthread_mutex_t mutex;

enum protocol {connectionRequest = 1, infoRequest, disconnectionRequest};


#pragma mark - Structures

typedef struct {
    struct sockaddr_in cli;
    //pthread_t tid;
    int s; /* Socket conectado ao cliente        */
} ClientParam;

typedef struct {
    struct hostent *ip_address;
    unsigned short port;
    char phone[PHONE_DIGITS];
} Client;

#pragma mark - Read & Write Functions

/**
 *  Searches for the next string in a given buffer.
 *
 *  @param buffer   Buffer in which the search is going to be performed on.
 *  @param position Initial position of the buffer where the search begins.
 *
 *  @return Returns the buffer position where the next string is. If there are no more strings in the buffer, returns -1.
 */
int getNextString(char buffer[TAM_BUF], int position) {
    while (buffer[position++] != '\0');
    if (buffer[position] == '#') return -1;
    return position;
}

/**
 *  Fetches a client by a given phone number.
 *
 *  @param phone Phone number of the client that is going to be fetched.
 *
 *  @return Returns the client if it was found, otherwise NULL.
 */
Client getClientByPhone(char phone[PHONE_DIGITS]) {
    
    //perform fread method to find the client;
    
    //if it finds a client, create a new client instance and returns it, else return NULL
    
    Client *client = (Client *)malloc(sizeof(Client));
//    client->ip_address = etc;
//    client->port = etc;
    client->phone = phone;
    
    return client;
}

/**
 *  Persists a client's information in file.
 *
 *  @param client The client that is going to be saved.
 *
 *  @return Returns 1 if operation succeeds, otherwise 0.
 */
bool saveClient(Client *client) {
    
    bool operationSucceed = 1;
    
    //fwrite method to save the client;
    
    if (error) {
        return !operationSucceed;
    }
    return operationSucceed;
}


#pragma mark - Other Functions



/**
 *  Method to be executed every time a new thread is created. A new thread is
 *  being created every time a new client requests for a connection.
 *
 *  @param client_connection Client connection pointer of type ClientParam.
 */
void *handle_client(void *client_connection) {
    
    /* Variaveis exclusivas da thread */
    socklen_t clientSocket;
    int l;
    char sendbuf[TAM_BUF], recvbuf[TAM_BUF], phone[PHONE_DIGITS];
    int i=0, j=0, tid;
    
    int bytesRead, connected = 1;
    
    ClientParam *par = (ClientParam *)(client_connection);
    par->cli.sin_port //source port
    par->cli.sin_addr.s_addr //source IP address
    clientSocket = par->s;
    
    //tid = par->tid;
    tid = pthread_self();
    struct sockaddr_in client = par->cli;
    printf("Thread[%u]: Cliente se conectou com %d\n", (unsigned)tid, clientSocket);
    while (connected) {
        printf("Thread[%u]: Aguardando mensagem do cliente\n", (unsigned)tid);
        if ((bytesRead = recv(clientSocket, recvbuf, sizeof(recvbuf), 0)) == -1)  {
            perror("Recv()");
            continue;
        }
        
        switch (recvbuf[protocol_index]) {
            case infoRequest: {
                printf("Thread[%u]: Cliente da porta %d deseja obter informações sobre outro usuário.\n", (unsigned)tid, ntohs(client.sin_port));
                //to-do: implementar este método
                break;
            }
            case connectionRequest: {
                printf("Thread[%u]: Cliente da porta %d deseja se conectar.\n", (unsigned)tid, ntohs(client.sin_port));
                l = 1;
                printf("O celular que esta se conectando eh: ");
                
                //to-do: método incompleto (não consegui terminar ainda - fique à vontade :P)
                
                do {
//                    printf("%-10s |  ",recvbuf+l);
                    printf("%s",recvbuf+l);
                    l = getNextString(recvbuf, l);
                    if (l == -1) break;
                    printf("Mensagem: %s.\n",recvbuf+l);
                    l = getNextString(recvbuf,l);
                } while (l != -1);
                
                break;
            }
            case disconnectionRequest: {
                printf("Thread[%u]: Cliente da porta %d deseja se desconectar.\n", (unsigned)tid, ntohs(client.sin_port));
                connected = 0;
                break;
            }
            default: {
                printf("Thread[%u]: Comando de código %d desconhecido. Enviando resposta ao cliente da porta %d\n", (unsigned)tid, recvbuf[protocol_index], ntohs(client.sin_port));
                //to-do: implementar este método
//                sendResp(sendbuf, clientSocket, invalido);
                break;
            }
        }
    }
    pthread_exit(0);
}




#pragma mark - Main

int main(int argc, const char * argv[]) {
    
    unsigned short port;
    struct sockaddr_in client;
    struct sockaddr_in server;
    socklen_t s;                     /* Socket para aceitar conexões */
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
    
    port = (unsigned short)atoi(argv[1]);
    
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
    printf("Bind ok. Preparando listen\n");
    
    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    
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
        ClientParam *p = (ClientParam *)malloc(sizeof(ClientParam));
        if ((p->s = accept(s, (struct sockaddr *)&p->cli, &namelen)) == -1) {
            perror("Accept()");
            exit(5);
        }
        
        printf("\nCriando thread de atendimento para o cliente na porta %d, handler %d\n", ntohs(p->cli.sin_port), p->s);
        
        pthread_create(&thread_id, NULL, handle_client, (void *)p); //cria a thread
        
        /**
         *  @author Roger Oba
         *
         *  Comentário/dúvida: acho que esse método (abaixo) não precisa ser chamado
         *  aqui, uma vez que ele já está sendo chamado na condicional abaixo. A
         *  documentação diz: "The effect of multiple pthread_detach() calls on the
         *  same target thread is unspecified."
         *
         *  Não quero arriscar quebrar o código, houve testes quanto à isso? Quem
         *  escreveu essa parte do código, por que deu detach duas vezes?
         */
        pthread_detach(thread_id);
        
        
        if ((int *)thread_id > 0)  {
            printf("Thread filha criada: %u\n", (unsigned) thread_id);
            pthread_detach(thread_id);
        } else {
            perror("Thread creation!");
            exit(7);
        }
    }
    
    //isso nunca é executado
    printf("Servidor terminou com sucesso.\n");
    exit(0);
}
