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
#include <stdbool.h>
#include <arpa/inet.h>

#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */

#include <unistd.h> // for close
#include <sys/wait.h> // wait
#include <pthread.h>


/////////// - Global Variables
#define PORT_SIZE 5
#define PHONE_SIZE 11
#define IP_SIZE 15
#define PROTOCOL_INDEX 0
#define MAX_ONLINE_USERS 256
#define BUFFMARGIN 40

pthread_t thread_id;
pthread_mutex_t mutex;

enum serverMessages
{
    CONNECTION_REQUEST = 1,
    UPDATE_REQUEST,
    DISCONNECT_REQUEST,
};


/////////// - Structures
struct SocketBuffer
{
    char *buffer;
    ssize_t pos;
    ssize_t size;
};


typedef struct Client Client;
struct Client {
    struct sockaddr_in client;
    int socket; //socket connected to the client
    char *listenIpAddress;
    unsigned short listenPort;
    char *phone;
    char *name;
    struct SocketBuffer buffer;
    int active;
    int readyForCommunication;
};

struct Client onlineClients[MAX_ONLINE_USERS];


/////////// - Read & Write Methods


void *buffRead(int size, struct SocketBuffer *buff)
{
    void *ptr;
    ptr = &buff->buffer[buff->pos];
    buff->pos+=size;
    if (buff->pos > buff->size-1)
        buff->pos = buff->size-1;
    return ptr;
}

void buffWrite(void *ptr, int size, struct SocketBuffer *buff)
{
    char *oldbuff;
    //Aloc the necessary amount of memory + margin
    if (buff->pos+size > buff->size)
    {
        oldbuff = buff->buffer;
        buff->buffer = (char*) malloc (buff->pos + size + BUFFMARGIN);
        memcpy(buff->buffer, oldbuff, buff->size);
        free(oldbuff);
        buff->size = buff->pos + size + BUFFMARGIN;
    }
    memcpy(&buff->buffer[buff->pos], ptr, size);
    buff->pos+=size;
}

unsigned char readByte(struct SocketBuffer *buff)
{
    return (*(unsigned char*)buffRead(sizeof(char), buff));
}

int readInt(struct SocketBuffer *buff)
{
    return (*(int*)buffRead(sizeof(int), buff));
}

int readShort(struct SocketBuffer *buff)
{
    return (*(short*)buffRead(sizeof(short), buff));
}

double readDouble(struct SocketBuffer *buff)
{
    return (*(double*)buffRead(sizeof(double), buff));
}

char *readString(struct SocketBuffer *buff)
{
    char now;
    int initialPos=buff->pos;
    int size;
    char *retval;
    
    now = buff->buffer[buff->pos];
    while(now != '\0' && buff->pos < buff->size)
    {
        buff->pos++;
        now = buff->buffer[buff->pos];
    }
    
    size = buff->pos-initialPos;
    retval = (char*)malloc(size);
    memcpy(retval, &buff->buffer[initialPos], size);
    buff->pos++;
    return retval;
}

void writeInt(int value, struct SocketBuffer *buff)
{
    buffWrite(&value, sizeof(int), buff);
}

void writeShort(short value, struct SocketBuffer *buff)
{
    buffWrite(&value, sizeof(short), buff);
}

void writeDouble(double value, struct SocketBuffer *buff)
{
    buffWrite(&value, sizeof(double), buff);
}

void writeByte(unsigned char byte, struct SocketBuffer *buff)
{
    buffWrite(&byte, sizeof(unsigned char), buff);
}

void writeString(char *string, struct SocketBuffer *buff)
{
    int size = 0;
    int strChar = string[0];
    while(strChar!='\0')
    {
        size++;
        strChar = string[size];
    }
    size++;
    
    buffWrite(string, size, buff);
}

void clearBuffer(struct SocketBuffer *buff)
{
    buff->pos = 0;
}
void startBuffer(struct SocketBuffer *buff)
{
    buff->size = 1;
    buff->buffer = (char*)malloc(1);
}
void closeBuffer(struct SocketBuffer *buff)
{
    if (buff->buffer!=NULL)
    {
        free(buff->buffer);
    }
}

//@ns = socket descriptor
void sendResp(struct SocketBuffer *buff, int ns)
{
    ssize_t totalToSend, totalSent, sentBytes;
    struct SocketBuffer sendBuff;
    
    startBuffer(&sendBuff);
    clearBuffer(&sendBuff);
    writeInt(buff->pos, &sendBuff);
    buffWrite((void*)buff->buffer, buff->pos, &sendBuff);
    
    totalToSend = sendBuff.pos;
    totalSent = 0;
    
    while(totalSent < totalToSend)
    {
        sentBytes = send(ns, sendBuff.buffer + totalSent, (totalToSend-totalSent), 0);
        if (sentBytes < 0)
        {
            perror("Send()");
            return;
        }
        totalSent += sentBytes;
    }
}

//@ns = socket descriptor
void recvResp(struct SocketBuffer *buff, int ns)
{
	char *oldbuff;
    ssize_t bytesToReceive = 0, bytesReceived, totalBytesReceived = 0, sizeToReceive = 0;
    clearBuffer(buff);
    
    sizeToReceive = sizeof(int);
    
    //recebe tamanho do pacote
    while (totalBytesReceived < sizeToReceive)
    {
        bytesReceived = recv(ns, &bytesToReceive + totalBytesReceived, sizeToReceive - totalBytesReceived, 0);
        
        if (bytesReceived  == -1)
        {
            perror("Recv()");
            exit(6);
        }
        totalBytesReceived += bytesReceived;
    }
    
    //Aloca quantidade de espaco necessaria + margem (pra evitar call pro malloc o tempo todo)
    if (buff->pos+bytesToReceive > buff->size)
    {
        oldbuff = buff->buffer;
        buff->buffer = (char*) malloc (buff->pos + bytesToReceive + BUFFMARGIN);
        memcpy(buff->buffer, oldbuff, buff->size);
        free(oldbuff);
        buff->size = buff->pos + (int)bytesToReceive + BUFFMARGIN;
    }
    
    //recebe pacote
    totalBytesReceived = 0;
    bytesReceived = 0;
    while (bytesReceived < bytesToReceive)
    {
        bytesReceived = recv(ns, buff->buffer + totalBytesReceived, bytesToReceive - totalBytesReceived, 0);
        if (bytesReceived  == -1)
        {
            perror("Recv()");
            exit(6);
        }
        totalBytesReceived += bytesReceived;
    }
}


void clearClient(Client *client)
{
    client->socket = -1;
    client->listenPort = -1;
    startBuffer(&(client->buffer));
    clearBuffer(&(client->buffer));
    client->active=0;
    client->readyForCommunication = 0;
}

void disconnectClient(Client *client)
{
//    shutdown(client->socket);
	client->readyForCommunication = 0;
    close(client->socket);
    free(client->phone);
    free(client->name);
    free(client->listenIpAddress);
    client->socket = -1;
    client->listenPort = -1;
    startBuffer(&(client->buffer));
    clearBuffer(&(client->buffer));
	client->active=0;
}
/////////// - Other Functions

/**
 *  Method to be executed every time a new thread is created. A new thread is
 *  being created every time a new client requests for a connection.
 *
 *  @param client_connection Client connection pointer of type Client.
 */

void *updateClient(void *myClientarg)
{
    int myTimer=0,i=0;
    int myClient = (*(int*)myClientarg);
    struct SocketBuffer buff;
    struct SocketBuffer *buffer = &buff;
    startBuffer(buffer);
    clearBuffer(buffer);
    while(onlineClients[myClient].readyForCommunication==1)
    {
        myTimer++;
        if (myTimer==10000)
        {
            int onlineUsers=0;
            for(i=0;i<MAX_ONLINE_USERS; i++)
            {
                if (onlineClients[i].readyForCommunication==1)
                {
                    onlineUsers++;
                }
            }
            clearBuffer(buffer);
            writeByte(UPDATE_REQUEST, buffer);//MESSAGE ID
            writeInt(onlineUsers, buffer);
            for(i=0;i<MAX_ONLINE_USERS; i++)
            {
                if (onlineClients[i].readyForCommunication==1)
                {
                    writeString(onlineClients[i].name, buffer);
                    writeString(onlineClients[i].phone, buffer);
                    writeString(onlineClients[i].listenIpAddress, buffer);
                    writeShort(onlineClients[i].listenPort, buffer);
                }
            }
            sendResp(buffer, onlineClients[myClient].socket);
        }
    }
    closeBuffer(buffer);
    pthread_exit(0);
}



void *handle_client(void *threadClientIdarg) {
    
    
    int threadClientId = (*(int*)threadClientIdarg);
    free(threadClientIdarg);
    
    /* Variaveis exclusivas da thread */
    socklen_t clientSocket;
    int messageid, connected = 1, i = 0;

    pthread_t tid = pthread_self();
    
//    unsigned short sourcePort = onlineClients[threadClientId].client.sin_port; //source port
//    struct in_addr sourceIP = onlineClients[threadClientId].client.sin_addr; //source IP address
    clientSocket = onlineClients[threadClientId].socket;
    struct sockaddr_in client = onlineClients[threadClientId].client;
    struct SocketBuffer *buffer = &(onlineClients[threadClientId].buffer);
    
    printf("Thread[%u]: Cliente se conectou com %d\n", (unsigned)tid, clientSocket);
    
    pthread_create(&thread_id, NULL, updateClient, (void *)&threadClientId); //cria a thread
    
    while (connected)
    {
        printf("Thread[%u]: Aguardando mensagem do cliente\n", (unsigned)tid);
        
        recvResp(buffer, onlineClients[threadClientId].socket);
        
        messageid = readByte(buffer);
        
        switch (messageid)
        {
            case UPDATE_REQUEST:
            {
                break;
            }
            case CONNECTION_REQUEST:
            {
                printf("Thread[%u]: Cliente da porta %d deseja se conectar.\n", (unsigned)tid, ntohs(client.sin_port));
                
                onlineClients[threadClientId].name = readString(buffer);
                onlineClients[threadClientId].phone = readString(buffer);
                onlineClients[threadClientId].listenIpAddress = readString(buffer);
                onlineClients[threadClientId].listenPort = readShort(buffer);
                onlineClients[threadClientId].readyForCommunication = 1;
                
                printf("O celular que esta se conectando eh: %s com usuario: %s", onlineClients[threadClientId].phone,onlineClients[threadClientId].name);
                //envia confirmacao de conexao
                clearBuffer(buffer);
                writeByte(CONNECTION_REQUEST,buffer);
                sendResp(buffer, clientSocket);
                
                break;
            }
            case DISCONNECT_REQUEST:
            {
                printf("Thread[%u]: Cliente da porta %d esta se desconectando.\n", (unsigned)tid, ntohs(client.sin_port));
                disconnectClient(&onlineClients[threadClientId]);
                break;
            }
            default:
            {
                printf("Thread[%u]: Comando de código %d desconhecido. Enviando resposta ao cliente da porta %d\n", (unsigned)tid, messageid, ntohs(client.sin_port));
                //to-do: implementar este método
                //                sendResp(sendbuf, clientSocket, invalido);
                break;
            }
        }
    }
    pthread_exit(0);
}

/////////// - Main

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
    
    /*b
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
    printf("Listen ok. Chamando accept\n");
    
    /*
     * Aceita uma conexão e cria um novo socket através do qual
     * ocorrerá a comunicação com o cliente.
     */
    namelen = sizeof(client);
    
    int nextClientId = -1;
    int i=0;
    
    for(i=0;i<MAX_ONLINE_USERS; i++)
    {
        clearClient(&onlineClients[i]);
    }
    
    while (1)
    {
        
        printf("Servidor pronto e aguardando novo cliente\n");
        for(i=0;i<MAX_ONLINE_USERS; i++)
        {
            if (onlineClients[i].active==0)
            {
                nextClientId=i;
                break;
            }
        }
        
        clearClient(&onlineClients[nextClientId]);
        
        if ((onlineClients[nextClientId].socket = accept(s, (struct sockaddr *)&(onlineClients[nextClientId].client), &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }
        
        onlineClients[nextClientId].active = 1;
        int *threadClientId = (int*) malloc (sizeof(int));
        (*threadClientId) = nextClientId;
        
        printf("\nCriando thread de atendimento para o cliente na porta %d, handler %d\n", ntohs(onlineClients[nextClientId].client.sin_port), onlineClients[nextClientId].socket);
        
        pthread_create(&thread_id, NULL, handle_client, (void *)threadClientId); //cria a thread
        
        nextClientId = -1;
        
        if ((int *)thread_id > 0)  {
            printf("Thread filha criada: %u\n", (unsigned) thread_id);
            pthread_detach(thread_id);
        } else {
            perror("Thread creation!");
            exit(7);
        }
    }
    
    
}
