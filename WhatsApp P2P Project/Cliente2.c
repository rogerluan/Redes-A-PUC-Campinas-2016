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
#include <netdb.h>


#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */

#include <unistd.h> // for close
#include <sys/wait.h> // wait
#include <pthread.h>

#include <ifaddrs.h>


/////////// - Global Variables
#define PORT_SIZE 5
#define PHONE_SIZE 11
#define IP_SIZE 15
#define PROTOCOL_INDEX 0
#define MAX_ONLINE_USERS 256
#define BUFFMARGIN 40
#define LASTMESSAGE 4
#define MAXGROUPCONTACT 10
#define MAXGROUPS 500

pthread_t thread_id;
pthread_mutex_t mutex;

enum serverMessages
{
    CONNECTION_REQUEST = 1,
    UPDATE_REQUEST,
    DISCONNECT_REQUEST,
};

enum clientMessages
{
    TEXT_MESSAGE = 1,
    IMAGE_MESSAGE
};

enum clientOperations
{
    OP_ADDCONTACT = 1,
    OP_MAKEGROUP,
    OP_SENDMESSAGE,
    OP_SENDIMAGE,
    OP_UPDATESCREEN,
    OP_LEAVE
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

struct Contact
{
    char name[50];
    char phone[PHONE_SIZE];
};
struct ContactGroup
{
    struct Contact contacts[MAXGROUPCONTACT];
    int size;
    int active;
};



struct ContactGroup myGroups[MAXGROUPS];
struct Client onlineClients[MAX_ONLINE_USERS];
struct Client serverOnlineClients[MAX_ONLINE_USERS];

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

void buffWrite(void *ptr, ssize_t size, struct SocketBuffer *buff)
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
    ssize_t initialPos=buff->pos;
    ssize_t size;
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
    if (buff->pos > buff->size-1)
        buff->pos = buff->size-1;
    return retval;
}

char *readFile(int size,struct SocketBuffer *buff)
{
    char *newFile;
    newFile = (char*)malloc(size);
    memcpy(newFile,&(buff->buffer[buff->pos]),size);
    buff->pos+=size;
    if (buff->pos > buff->size-1)
        buff->pos = buff->size-1;
    return newFile;
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
    writeInt((int)buff->pos, &sendBuff);
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
        buff->size = buff->pos + bytesToReceive + BUFFMARGIN;
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
    closeBuffer(&(client->buffer));
    client->active=0;
}

void clearServerClientInfo(Client *client)
{
    client->readyForCommunication = 0;
    free(client->phone);
    free(client->name);
    free(client->listenIpAddress);
    client->listenPort = -1;
    startBuffer(&(client->buffer));
    clearBuffer(&(client->buffer));
    closeBuffer(&(client->buffer));
    client->active=0;
}

/////////// - Shared Vars
int port;
char myIp[16];
socklen_t serverSocket;
int operational;
int usersOnline=0;
char lastMessages[LASTMESSAGE+1][140];
int dontStartOperations;
/////////// - Other Functions

void printMenu() {
    //improves readability
    int i=0;
    printf("\033[2J\033[1;1H");//clear screen
    printf("Messages: \n");
    for(i=0;i<LASTMESSAGE+1;i++)
    {
        printf("%s",lastMessages[i]);
    }
    printf("\n**********************************\n");
    printf("Opcoes:\n");
    printf("1 - Adicionar Contato\n");
    printf("2 - Criar Grupo de Broadcast\n");
    printf("3 - Enviar Mensagem\n");
    printf("4 - Enviar Foto\n");
    printf("5 - Atualizar Tela\n");
    printf("6 - Sair\n");
    printf("**********************************\n");
}

/**
 *  Method to be executed every time a new thread is created. A new thread is
 *  being created every time a new client requests for a connection.
 *
 *  @param client_connection Client connection pointer of type Client.
 */
void *handle_client(void *threadClientIdarg)
{
    
    int threadClientId = (*(int*)threadClientIdarg);
    free(threadClientIdarg);
    
    /* Variaveis exclusivas da thread */
    socklen_t clientSocket;
    int connected = 1;
    pthread_t tid = pthread_self();
    
    clientSocket = onlineClients[threadClientId].socket;
    struct sockaddr_in client = onlineClients[threadClientId].client;
    struct SocketBuffer *buffer = &(onlineClients[threadClientId].buffer);
    
    int messageid;
    int i=0;
    printf("Thread[%u]: Cliente se conectou com %d\n", (unsigned)tid, clientSocket);
    char *sender,*phone,*msg,*file;
    int fileSize=0;
    while (connected)
    {
        //printf("Thread[%u]: Aguardando mensagem do cliente\n", (unsigned)tid);
        
        recvResp(buffer, onlineClients[threadClientId].socket);
        
        messageid = readByte(buffer);
        
        switch (messageid)
        {
            case TEXT_MESSAGE:
                
                sender = readString(buffer);
                phone = readString(buffer);
                msg = readString(buffer);
                for(i=1;i<LASTMESSAGE+1;i++)
                {
                    strcpy(lastMessages[i-1],lastMessages[i]);
                }
                strcpy(lastMessages[LASTMESSAGE],"");
                strcat(lastMessages[LASTMESSAGE],sender);
                strcat(lastMessages[LASTMESSAGE]," / ");
                strcat(lastMessages[LASTMESSAGE],phone);
                strcat(lastMessages[LASTMESSAGE],":\n");
                strcat(lastMessages[LASTMESSAGE],msg);
                strcat(lastMessages[LASTMESSAGE],"\n");
                free(sender);
                free(phone);
                free(msg);
                //printMenu();
                break;
            case IMAGE_MESSAGE:
                sender = readString(buffer);
                phone = readString(buffer);
                fileSize = readInt(buffer);
                file = readFile(fileSize,buffer);
                
                FILE *diskFile= fopen("photo.png","wb");
                if (diskFile!=NULL)
                {
                    fwrite(file,0,fileSize,diskFile);
                    fclose(diskFile);
                }
                
                for(i=1;i<LASTMESSAGE+1;i++)
                {
                    strcpy(lastMessages[i-1],lastMessages[i]);
                }
                strcpy(lastMessages[LASTMESSAGE],"");
                strcat(lastMessages[LASTMESSAGE],sender);
                strcat(lastMessages[LASTMESSAGE]," / ");
                strcat(lastMessages[LASTMESSAGE],phone);
                strcat(lastMessages[LASTMESSAGE],"Image Received, saved in:");
                strcat(lastMessages[LASTMESSAGE],"photo.png");
                strcat(lastMessages[LASTMESSAGE],"\n");
                
                free(file);
                free(sender);
                free(phone);
                break;
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

void *serverReceiver(void *param)
{
    struct SocketBuffer myBuff;
    startBuffer(&myBuff);
    clearBuffer(&myBuff);
    
    int messageid=0;
    int i=0;
    while(operational)
    {
        recvResp(&myBuff,serverSocket);
        messageid = readByte(&myBuff);
        switch (messageid)
        {
            case CONNECTION_REQUEST:
                printf("Sucessfuly connected to the server.");
                dontStartOperations=0;
                break;
            
            case UPDATE_REQUEST:
                
                for (i=0;i<MAX_ONLINE_USERS;i++)
                {
                    if (serverOnlineClients[i].readyForCommunication!=0)
                    {
                        clearServerClientInfo(&serverOnlineClients[i]);
                    }
                }
                
                usersOnline = readInt(&myBuff);
                
                for (i=0;i<usersOnline;i++)
                {
                    serverOnlineClients[i].name = readString(&myBuff);
                    serverOnlineClients[i].phone = readString(&myBuff);
                    serverOnlineClients[i].listenIpAddress = readString(&myBuff);
                    serverOnlineClients[i].listenPort = readShort(&myBuff);
                    serverOnlineClients[i].readyForCommunication = 1;
                }
                break;
        
            case DISCONNECT_REQUEST:
                
                break;
                
            default:
                printf("Message not recognized. \n");
                break;
        
        }
    }
    
    closeBuffer(&myBuff);
    pthread_exit(0);

}

void *clientOperation(void *param)
{
    int option,i, choice, tempSize;
    char nome[50],phone[PHONE_SIZE];
    struct SocketBuffer myBuff;
    startBuffer(&myBuff);
    clearBuffer(&myBuff);
    printf("Digite seu nome:\n");
    gets(nome);
    printf("Digite seu telefone:\n");
    gets(phone);
    
    startBuffer(&myBuff);
    clearBuffer(&myBuff);
    writeByte(CONNECTION_REQUEST,&myBuff);
    writeString(nome,&myBuff);
    writeString(phone,&myBuff);
    writeString(myIp,&myBuff);
    writeShort(port,&myBuff);
    sendResp(&myBuff,serverSocket);
    
    while(dontStartOperations)
    {
      //espera ateh receber ack pela thread de recv
    }
    printf("Successfuly connected to the server.");
    //serverSocket
    while (operational)
    {
        printMenu();
        scanf("%d", &option);
        switch(option)
        {
            case OP_ADDCONTACT:
                printf("ID - Nome - Phone");
                for (i=0;i<usersOnline;i++)
                {
                    printf("%d - %s - %s",i,serverOnlineClients[i].name,serverOnlineClients[i].phone);
                }
                printf("Entre com o ID do usuario que gostaria de adicionar");
                scanf("%d", &choice);
                if (choice>usersOnline)
                {
                    printf("id invalido\n");
                    continue;
                }
                if (serverOnlineClients[choice].readyForCommunication==0)
                {
                    printf("id invalido\n");
                    continue;
                }
                
                for (i=0;i<MAXGROUPS;i++)
                {
                   if (myGroups[i].active==0)
                   {
                       strcpy(myGroups[i].contacts[0].name, serverOnlineClients[choice].name);
                       strcpy(myGroups[i].contacts[0].phone, serverOnlineClients[choice].phone);
                       myGroups[i].size = 1;
                       myGroups[i].active = 1;
                       break;
                   }
                }
                printf("Contato Adicionado!\n");
                //Envia msg pedindo users
                
                break;
            case OP_MAKEGROUP:
                
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==1 && myGroups[i].size==1)
                    {
                        printf("%d %s %s",i,myGroups[i].contacts[0].name,myGroups[i].contacts[0].phone);
                    }
                }
                tempSize=0;
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==0)
                    {
                        choice = 0;
                        printf("Digite o id do usuario que quer adicionar ao novo grupo, digite -1 para voltar\n");
                        while(choice!=-1 && tempSize < MAXGROUPCONTACT)
                        {
                            scanf("%d",&choice);
                            if (choice>usersOnline)
                            {
                                printf("id invalido\n");
                                continue;
                            }
                            if (serverOnlineClients[choice].readyForCommunication==0)
                            {
                                printf("id invalido\n");
                                continue;
                            }
                            if (choice!= -1)
                            {
                                strcpy(myGroups[i].contacts[tempSize].name, myGroups[choice].contacts[0].name);
                                strcpy(myGroups[i].contacts[tempSize].phone, myGroups[choice].contacts[0].phone);
                                tempSize++;
                                printf("Digite o id do proximo usuario, digite -1 para voltar\n");
                            }
                        }
                        if (tempSize>0)
                            myGroups[i].active = 1;
                        
                    }
                }
                
                
                break;
            case OP_SENDMESSAGE:
                //Envia msg pedindo users
                
                break;
            case OP_SENDIMAGE:
                //Envia msg pedindo users
                
                break;
            case OP_UPDATESCREEN:
                //Envia msg pedindo users
                
                break;
            case OP_LEAVE:
                //Envia msg pedindo users
                
                break;
            default:
                printf("Erro, comando nao reconhecido");
                break;
                
        }
        
    }
    closeBuffer(&myBuff);
    pthread_exit(0);
}

/////////// - Main

int main(int argc, const char * argv[])
{
    unsigned short port;
    unsigned short portServer;
    struct sockaddr_in client;
    struct sockaddr_in server;
    struct sockaddr_in server2;
    struct hostent *hostnm;
    
    socklen_t s;                     /* Socket para aceitar conexões */
    socklen_t namelen;
    //signal(SIGCHLD,receive_child_signal);
    
    int i=0;
    operational = 1;
    dontStartOperations = 1;
    for(i=0;i<MAX_ONLINE_USERS; i++)
    {
        clearClient(&onlineClients[i]);
    }
    for(i=0;i<LASTMESSAGE+1;i++)
    {
        strcpy(lastMessages[i],"\n");
    }
    for(i=0;i<MAXGROUPS;i++)
    {
        myGroups[i].active = 0;
        myGroups[i].size = 0;
    }
    
    
    pthread_mutex_init(&mutex, NULL);
    
    //============================================================================================
    //ESCOLHE PORTA E PEGA IP
    port = (unsigned short)rand() %40000 + 20000; //Portas de 20000 a 60000
    fprintf(stderr, "Ouvindo Porta: %d porta\n", port);
    
    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }
    
    struct ifaddrs *tmp,*addrs;
    getifaddrs(&addrs);
    tmp = (addrs);
    
    int jumpFirst = 0;
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            if (jumpFirst==1)
            {
                strcpy(myIp,inet_ntoa(pAddr->sin_addr));
                printf("Meu IP: %s\n", myIp);
                break;
            }
            jumpFirst++;
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(addrs);
    
    //============================================================================================
    
    //CRIA THREAD DE CLIENTE
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }
    /*
     * Obtendo o endereço IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    portServer = (unsigned short) atoi(argv[2]);
    
    /*
     * Define o endereço IP e a porta do servidor
     */
    server2.sin_family      = AF_INET;
    server2.sin_port        = htons(portServer);
    server2.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
    
    /*
     * Cria um socket TCP (stream)
     */
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }
    
    /* Estabelece conexão com o servidor */
    if (connect(serverSocket, (struct sockaddr *)&server2, sizeof(server2)) < 0)
    {
        perror("Connect()");
        exit(4);
    }
    
    pthread_create(&thread_id, NULL, clientOperation, NULL); //cria a thread de operacoes do usuario
    pthread_create(&thread_id, NULL, serverReceiver, NULL); //cria a thread de recebr dados do server

    //============================================================================================
    
    
    //CRIA COISAS DE SERVIDOR
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
   // printf("Bind ok. Preparando listen\n");
    
    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
   // printf("Listen ok. Chamando accept\n");
    
    /*
     * Aceita uma conexão e cria um novo socket através do qual
     * ocorrerá a comunicação com o cliente.
     */
    namelen = sizeof(client);
    
    int nextClientId = -1;
    
    
    while (1)
    {
    //    printf("Servidor pronto e aguardando novo cliente\n");
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
        
        if ((int *)thread_id > 0)  
        {
            printf("Thread filha criada: %u\n", (unsigned) thread_id);
            pthread_detach(thread_id);
        } else {
            perror("Thread creation!");
            exit(7);
        }
    }
    
    
}
