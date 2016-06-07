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

#pragma mark - Global Variables

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

int port;
char myIp[16];
socklen_t serverSocket;
int operational;
int usersOnline=0;
char lastMessages[LASTMESSAGE+1][140];
int dontStartOperations;
char myName[50], myPhone[PHONE_SIZE];

#pragma mark - Enumerations

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

#pragma mark - Structures

struct SocketBuffer
{
    char *buffer;
    ssize_t pos;
    ssize_t size;
};

struct P2Message
{
    struct SocketBuffer *buffer;
    char listenIpAddress[15];
    unsigned short listenPort;
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

#pragma mark - Read & Write Methods

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
    //Alloc the necessary amount of memory + margin
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

char *readFile(int size, struct SocketBuffer *buff)
{
    char *newFile;
    newFile = (char*)malloc(size);
    memcpy(newFile, &(buff->buffer[buff->pos]), size);
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

#pragma mark - Other Functions

void printMenu() {
    //improves readability
    int i=0;
    printf("\033[2J\033[1;1H");//clear screen
    printf("Messages: \n");
    for(i=0;i<LASTMESSAGE+1;i++)
    {
        printf("%s", lastMessages[i]);
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
 *  @param threadClientIdarg Thread client ID
 */
void *handle_client(void *threadClientIdarg)
{
    
    int threadClientId = (*(int*)threadClientIdarg);
    free(threadClientIdarg);
    
    /* Variaveis exclusivas da thread */
    socklen_t clientSocket;
    pthread_t tid = pthread_self();
    
    clientSocket = onlineClients[threadClientId].socket;
//    struct sockaddr_in client = onlineClients[threadClientId].client;
    struct SocketBuffer *buffer = &(onlineClients[threadClientId].buffer);
    
    int messageid;
    int i=0;
    //printf("Thread[%u]: Cliente se conectou com %d\n", (unsigned)tid, clientSocket);
    char *sender, *phone, *msg, *file;
    int fileSize=0;
    
    recvResp(buffer, onlineClients[threadClientId].socket);
    
    messageid = readByte(buffer);
    
    switch (messageid)
    {
        case TEXT_MESSAGE:
            
            sender = readString(buffer);
            phone = readString(buffer);
            msg = readString(buffer);
            //printf("%s %s %s\n",sender,phone,msg);
            for(i=1;i<LASTMESSAGE+1;i++)
            {
                strcpy(lastMessages[i-1], lastMessages[i]);
            }
            strcpy(lastMessages[LASTMESSAGE], "");
            strcat(lastMessages[LASTMESSAGE], sender);
            strcat(lastMessages[LASTMESSAGE], " / ");
            strcat(lastMessages[LASTMESSAGE], phone);
            strcat(lastMessages[LASTMESSAGE], ":\n");
            strcat(lastMessages[LASTMESSAGE], msg);
            strcat(lastMessages[LASTMESSAGE], "\n");
            free(sender);
            free(phone);
            free(msg);
            //printMenu();
            break;
        case IMAGE_MESSAGE:
            ;char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            sender = readString(buffer);
            phone = readString(buffer);
            fileSize = readInt(buffer);
            printf("FileSize to Receive: %d\n",fileSize);
            file = readFile(fileSize, buffer);
            
            FILE *diskFile= fopen("photorecv.png", "wb");
            if (diskFile!=NULL)
            {
                fwrite(file, 1,fileSize, diskFile);
                fclose(diskFile);
            }else
            {
                printf("error salvando arquivo\n");
            }
            
            for(i=1;i<LASTMESSAGE+1;i++)
            {
                strcpy(lastMessages[i-1], lastMessages[i]);
            }
            strcpy(lastMessages[LASTMESSAGE], "");
            strcat(lastMessages[LASTMESSAGE], sender);
            strcat(lastMessages[LASTMESSAGE], " / ");
            strcat(lastMessages[LASTMESSAGE], phone);
            strcat(lastMessages[LASTMESSAGE], "Image Received, saved in:");
            strcat(lastMessages[LASTMESSAGE], cwd);
            strcat(lastMessages[LASTMESSAGE], "/");
            strcat(lastMessages[LASTMESSAGE], "photorecv.png");
            strcat(lastMessages[LASTMESSAGE], "\n");
            
            free(file);
            free(sender);
            free(phone);
            break;
        default:
        {
            printf("Thread[%u]: Comando de código %d desconhecido.\n", (unsigned)tid, messageid);
            break;
        }
    }
    usleep(5000000);
    disconnectClient(&onlineClients[threadClientId]);
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
        recvResp(&myBuff, serverSocket);
        messageid = readByte(&myBuff);
        switch (messageid)
        {
            case CONNECTION_REQUEST:
                printf("Sucessfuly connected to the server.\n");
                dontStartOperations=0;
                break;
                
            case UPDATE_REQUEST:
                //printf("Receiving Users.\n");
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
                    //printf("%s, %s, %s, %hu\n",serverOnlineClients[i].name,serverOnlineClients[i].phone,serverOnlineClients[i].listenIpAddress,serverOnlineClients[i].listenPort);
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

/**
 *  Persists a buffer information in file.
 *
 *  @param buffer The buffer that is going to be saved.
 *
 *  @return Returns 1 if operation succeeds, otherwise 0.
 */
bool saveBufferToFile(struct SocketBuffer *buffer)
{
    bool operationSucceed = true, triedCreating = false;
    
    if (!buffer || buffer == NULL) {
        return !operationSucceed;
    }
    
    FILE *contactsBook;
    
    do {
        //try to open the contacts book file
        contactsBook = fopen("contactsBook.amiguinhos", "r+");
        
        if (!contactsBook) { //unsuccess case
            // try to create the contacts book file
            if (!triedCreating) {
                triedCreating = true;
                fclose(fopen("contactsBook.amiguinhos", "w"));
            } else {
                printf("Error opening file.");
                return !operationSucceed;
            }
        } else {
            break; //check if this breaks from if or from loop
            //triedCreating = false; //if it breaks only from if, uncomment this line.
        }
    } while (triedCreating);
    
    fwrite(buffer->buffer, 1, buffer->size, contactsBook);
    
    if (contactsBook) {
        fclose(contactsBook);
    }
    
    return operationSucceed;
}

/*!
 *  Reads a previously saved contacts file, and returns the information in a buffer.
 *
 *  @return Returns a buffer containing the groups read from the file, or null if no groups were found.
 */
struct SocketBuffer readGroupsFromFile()
{
    struct SocketBuffer returnBuffer;
    startBuffer(&returnBuffer);
    clearBuffer(&returnBuffer);
    
    FILE *diskFile = fopen("contactsBook.amiguinhos", "rb");
    size_t fileSize;
    if (diskFile!=NULL)
    {
        fseek(diskFile, 0L, SEEK_END);
        fileSize = ftell(diskFile);
        fclose(diskFile);
        
        printf("File size: %zu\n", fileSize);
        char *charFile;
        charFile = (char *)malloc(fileSize);
        
        diskFile = fopen("contactsBook.amiguinhos", "rb");
        if (diskFile!=NULL)
        {
            fread(charFile, 1, fileSize, diskFile);
            fclose(diskFile);
            
            returnBuffer.buffer = charFile;
            returnBuffer.size = fileSize;
            
            free(charFile);
        } else {
            printf("Livro de contatos invalido.\n");
            usleep(1000000);
        }
    } else {
        printf("Livro de contatos invalido.\n");
        usleep(1000000);
    }
    
    return returnBuffer;
}

/*!
 *  Serialize the current contacts book into a file, to persist locally.
 */
void serializeGroupsToFile()
{
    struct SocketBuffer buffer;
    int i, k, numberOfGroups = 0;
    
    startBuffer(&buffer);
    clearBuffer(&buffer);
    
    for (i=0;i<MAXGROUPS;i++)
    {
        if (myGroups[i].active==1)
        {
            for (k=0;k<myGroups[i].size;k++)
            {
                numberOfGroups++;
            }
        }
    }
    
    writeInt(numberOfGroups, &buffer);
    
    for (i=0;i<MAXGROUPS;i++)
    {
        if (myGroups[i].active==1)
        {
            writeInt(myGroups[i].size, &buffer);
            for (k=0;k<myGroups[i].size;k++)
            {
                writeInt(myGroups[i].size, &buffer);
                writeString(myGroups[i].contacts[k].name, &buffer);
                writeString(myGroups[i].contacts[k].phone, &buffer);
            }
        }
    }
    saveBufferToFile(&buffer);
    closeBuffer(&buffer);
}

/*!
 *  Loads previously saved contacts and books
 */
void deserializeFileToGroups()
{
    struct SocketBuffer buffer;
    int i, k, numberOfGroups = 0;
    
    startBuffer(&buffer);
    clearBuffer(&buffer);
    buffer = readGroupsFromFile();
    
    if (!buffer.size) {
        return;
    }
    
    numberOfGroups = readInt(&buffer);
    
    for (i=0;i<numberOfGroups;i++)
    {
        if (myGroups[i].active==0)
        {
            myGroups[i].active = 1;
            myGroups[i].size = readInt(&buffer);
            for (k=0;k<myGroups[i].size;k++)
            {
                strcpy(myGroups[i].contacts[k].name, readString(&buffer));
                strcpy(myGroups[i].contacts[k].phone, readString(&buffer));
            }
        }
    }
}

/*!
 *  Connects to a given hostname and port.
 *
 *  @param hostnameParam Name of the host. This is automatically translated into IP address using gethostbyname().
 *  @param port          Port of the host.
 *
 *  @return Returns an int value of the socket used in the connection.
 */
int connectToServer(const char *hostnameParam, unsigned short port)
{
    
    struct hostent *hostname;
    struct sockaddr_in server;
    int returnSocket;
    
    //Obtains server IP address
    hostname = gethostbyname(hostnameParam);
    if (hostname == (struct hostent *)0) {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    
    //Defines server IP address and port.
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);
    
    //Creates socket
    if ((returnSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
        exit(3);
    }
    
    //Establishes a connection with the server.
    if (connect(returnSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect()");
        exit(4);
    }
    
    return returnSocket;
}

void *P2Sender(void * P2Messagearg)
{
    struct P2Message *message = (struct P2Message*)P2Messagearg;
    
    usleep(rand()%10000);
    //abre conexao tcp com message.listenport / message.listenip
    int newOpenedSocket = connectToServer(message->listenIpAddress, message->listenPort);
    
    sendResp(message->buffer, newOpenedSocket);
    usleep(30000000);
    close(newOpenedSocket);
    closeBuffer(message->buffer);
    free(message);
    
    pthread_exit(0);
}


void *clientOperation(void *param)
{
    int option, i, choice, tempSize;
    struct SocketBuffer myBuff;
    char text[100];
    struct P2Message *message;
    int k=0,choiceid=0;
    
    startBuffer(&myBuff);
    clearBuffer(&myBuff);
    printf("Digite seu nome:\n");
    gets(myName);
    printf("Digite seu telefone:\n");
    gets(myPhone);
    
    startBuffer(&myBuff);
    clearBuffer(&myBuff);
    writeByte(CONNECTION_REQUEST, &myBuff);
    writeString(myName, &myBuff);
    writeString(myPhone, &myBuff);
    writeString(myIp, &myBuff);
    writeShort(port, &myBuff);
    sendResp(&myBuff, serverSocket);
    
    deserializeFileToGroups();
    
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
            case OP_ADDCONTACT: {
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==0)
                    {
                        printf("Digite o nome do amiguinho \n");
                        fpurge(stdin);
                        gets(myGroups[i].contacts[0].name);
                        printf("Digite o telefone do amiguinho \n");
                        fpurge(stdin);
                        gets(myGroups[i].contacts[0].phone);
                        myGroups[i].size = 1;
                        myGroups[i].active = 1;
                        break;
                    }
                }
                printf("Contato Adicionado!\n");
                //Envia msg pedindo users
                
                break;
            }
            case OP_MAKEGROUP: {
                
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==1 && myGroups[i].size==1)
                    {
                        printf("%d %s %s", i, myGroups[i].contacts[0].name, myGroups[i].contacts[0].phone);
                    }
                }
                tempSize=0;
                choice = 0;
                printf("Digite o id do usuario que quer adicionar ao novo grupo, digite -1 para voltar\n");
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==0)
                    {
                        while(choice!=-1 && tempSize < MAXGROUPCONTACT)
                        {
                            scanf("%d", &choice);
                            if (choice>MAXGROUPS)
                            {
                                printf("id invalido\n");
                                continue;
                            }
                            if (myGroups[choice].active==0)
                            {
                                printf("id invalido\n");
                                continue;
                            }
                            if (choice!= -1)
                            {
                                strcpy(myGroups[i].contacts[tempSize].name, myGroups[choice].contacts[0].name);
                                strcpy(myGroups[i].contacts[tempSize].phone, myGroups[choice].contacts[0].phone);
                                myGroups[i].size++;
                                tempSize++;
                                printf("Digite o id do proximo usuario, digite -1 para voltar\n");
                            }
                        }
                        if (tempSize>0)
                        {
                            myGroups[i].active = 1;
                        }
                        break;
                    }
                }
                break;
            }
            case OP_SENDMESSAGE: {
                
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==1)
                    {
                        if (myGroups[i].size>1)
                        {
                            printf("//================\nGrupo ID: %d\n",i);
                            for(k=0;k<myGroups[i].size;k++)
                            {
                                printf("Nome: %s - Fone %s\n",myGroups[i].contacts[k].name,myGroups[i].contacts[k].phone);
                            }
                            printf("//================\n");
                        }
                        else
                        {
                            printf("Contato ID: %d -Nome: %s - Fone %s\n",i,myGroups[i].contacts[k].name,myGroups[i].contacts[k].phone);
                        }
                    }
                }
                printf("Escolha o ID pra quem enviar a mensagem:");
                scanf("%d",&choiceid);
                
                if (choiceid>MAXGROUPS)
                {
                    printf("Receptor Invalido\n");
                    break;
                }
                if (myGroups[choiceid].active!=1)
                {
                    printf("Receptor Invalido\n");
                    break;
                }
                
                printf("Digite sua mensagem:");
                fpurge(stdin);
                gets(text);
                
                for(k=0;k<myGroups[choiceid].size;k++)
                {
                    for(i=0;i<MAX_ONLINE_USERS;i++)
                    {
                        if (serverOnlineClients[i].readyForCommunication==1)
                        {
                            if (strcmp(myGroups[choiceid].contacts[k].phone,serverOnlineClients[i].phone)==0)
                            {
                                //found match
                                printf("Enviando mensagem para: %s\n",myGroups[choiceid].contacts[k].name);
                                
                                message = (struct P2Message*)malloc(sizeof(struct P2Message));
                                message->buffer = (struct SocketBuffer*)malloc(sizeof(struct SocketBuffer));
                                strcpy(message->listenIpAddress,serverOnlineClients[i].listenIpAddress);
                                message->listenPort = serverOnlineClients[i].listenPort;
                                startBuffer(message->buffer);
                                clearBuffer(message->buffer);
                                writeByte(TEXT_MESSAGE,message->buffer);
                                writeString(myName,message->buffer);
                                writeString(myPhone,message->buffer);
                                writeString(text,message->buffer);
                                
                                pthread_create(&thread_id, NULL, P2Sender, (void*)message);
                                
                                break;
                            }
                        }
                        if (i==MAX_ONLINE_USERS-1)
                        {
                            printf("Usuario: %s nao esta online\n",myGroups[choiceid].contacts[k].name);
                            break;
                        }
                    }
                }
                usleep(2000000);
                break;
            }
            case OP_SENDIMAGE: {
                
                for (i=0;i<MAXGROUPS;i++)
                {
                    if (myGroups[i].active==1)
                    {
                        if (myGroups[i].size>1)
                        {
                            printf("//================\nGrupo ID: %d\n",i);
                            for(k=0;k<myGroups[i].size;k++)
                            {
                                printf("Nome: %s - Fone %s\n",myGroups[i].contacts[k].name,myGroups[i].contacts[k].phone);
                            }
                            printf("//================\n");
                        }
                        else
                        {
                            printf("Contato ID: %d -Nome: %s - Fone %s\n",i,myGroups[i].contacts[k].name,myGroups[i].contacts[k].phone);
                        }
                    }
                }
                printf("Escolha o ID pra quem enviar a mensagem:");
                scanf("%d",&choiceid);
                
                if (choiceid>MAXGROUPS)
                {
                    printf("Receptor Invalido\n");
                    usleep(1000000);
                    break;
                }
                if (myGroups[choiceid].active!=1)
                {
                    printf("Receptor Invalido\n");
                    usleep(1000000);
                    break;
                }
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                printf("Digite o caminho do arquivo:\n");
                fpurge(stdin);
                gets(text);
                FILE *diskFile = fopen(text, "rb");
                size_t sz;
                if (diskFile!=NULL)
                {
                    fseek(diskFile, 0L, SEEK_END);
                    sz = ftell(diskFile);
                    fclose(diskFile);
                }else{
                    printf("arquivo invalido \n");
                    usleep(1000000);
                    break;
                }
                printf("File size: %zu\n",sz);
                char *file;
                file = (char*)malloc(sz);
                
                diskFile = fopen(text, "rb");
                if (diskFile!=NULL)
                {
                    fread(file, 1, sz ,diskFile);
                    fclose(diskFile);
                }else{
                    printf("arquivo invalido \n");
                    usleep(1000000);
                    break;
                }
                
                
                for(k=0;k<myGroups[choiceid].size;k++)
                {
                    for(i=0;i<MAX_ONLINE_USERS;i++)
                    {
                        if (serverOnlineClients[i].readyForCommunication==1)
                        {
                            if (strcmp(myGroups[choiceid].contacts[k].phone,serverOnlineClients[i].phone)==0)
                            {
                                //found match
                                printf("Enviando imagem para: %s\n",myGroups[choiceid].contacts[k].name);
                                
                                message = (struct P2Message*)malloc(sizeof(struct P2Message));
                                message->buffer = (struct SocketBuffer*)malloc(sizeof(struct SocketBuffer));
                                strcpy(message->listenIpAddress,serverOnlineClients[i].listenIpAddress);
                                message->listenPort = serverOnlineClients[i].listenPort;
                                startBuffer(message->buffer);
                                clearBuffer(message->buffer);
                                writeByte(IMAGE_MESSAGE,message->buffer);
                                writeString(myName,message->buffer);
                                writeString(myPhone,message->buffer);
                                writeInt((int)sz,message->buffer);
                                buffWrite(file,sz,message->buffer);
                                
                                pthread_create(&thread_id, NULL, P2Sender, (void*)message);
                                
                                break;
                            }
                        }
                        if (i==MAX_ONLINE_USERS-1)
                        {
                            printf("Usuario: %s nao esta online\n",myGroups[choiceid].contacts[k].name);
                            break;
                        }
                    }
                }
                usleep(10000000);
                free(file);
                break;
            }
            case OP_UPDATESCREEN: {
                //do nothing actually, screen will be redarawn on the top of this while
                break;
            }
            case OP_LEAVE: {
                serializeGroupsToFile();
            
                clearBuffer(&myBuff);
                writeByte(DISCONNECT_REQUEST, &myBuff);
                sendResp(&myBuff, serverSocket);
                
                break;
            }
            default: {
                printf("Erro, comando nao reconhecido");
                break;
            }
        }
        
    }
    closeBuffer(&myBuff);
    pthread_exit(0);
}

#pragma mark - Main

int main(int argc, const char * argv[])
{
    srand(time(NULL));
    struct sockaddr_in client;
    struct sockaddr_in server;
    socklen_t s;                     /* Socket para aceitar conexões */
    socklen_t namelen;
    
    int i=0;
    operational = 1;
    dontStartOperations = 1;
    for(i=0;i<MAX_ONLINE_USERS; i++)
    {
        clearClient(&onlineClients[i]);
    }
    for(i=0;i<LASTMESSAGE+1;i++)
    {
        strcpy(lastMessages[i], "\n");
    }
    for(i=0;i<MAXGROUPS;i++)
    {
        myGroups[i].active = 0;
        myGroups[i].size = 0;
    }
    
    
    pthread_mutex_init(&mutex, NULL);
    
    //============================================================================================
    //ESCOLHE PORTA E PEGA IP
    port = (unsigned short)rand()%40000 + 20000; //Portas de 20000 a 60000
    fprintf(stderr, "Ouvindo Porta: %d porta\n", port);
    
    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }
    
    struct ifaddrs *tmp, *addrs;
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
                strcpy(myIp, inet_ntoa(pAddr->sin_addr));
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
    
    serverSocket = connectToServer(argv[1], (unsigned short)atoi(argv[2]));
    
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
        
        //printf("\nCriando thread de atendimento para o cliente na porta %d, handler %d\n", ntohs(onlineClients[nextClientId].client.sin_port), onlineClients[nextClientId].socket);
        
        pthread_create(&thread_id, NULL, handle_client, (void *)threadClientId); //cria a thread
        
        nextClientId = -1;
        
        if ((int *)thread_id > 0)
        {
            //printf("Thread filha criada: %u\n", (unsigned) thread_id);
            pthread_detach(thread_id);
        } else {
            perror("Thread creation!");
            exit(7);
        }
    }
    
    
}
