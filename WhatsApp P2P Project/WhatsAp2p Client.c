//
//  WhatsAp2p Client.c
//  WhatsAp2p Client
//
//  Created by Roger Luan on 6/3/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for close
#include <stdbool.h>


#define TAM_BUF 400 //to-do
#define MAX_BUFFER_SIZE 2000
#define PHONE_SIZE 11
#define PORT_SIZE 5
#define MAX_NAME_SIZE 50

enum protocol {
    connectionRequest = 1,
    infoRequest,
    disconnectionRequest
};

enum userAction {
    userAction_AddContact = 1,
    userAction_CreateGroup,
    userAction_SendMessage,
    userAction_SendImage,
    userAction_Disconnect
};

typedef struct Contact Contact;
struct Contact {
//    struct sockaddr_in client;
//    int socket; //socket connected to the client
    struct in_addr ip_address;
    unsigned short port;
    char phone[PHONE_SIZE], name[MAX_NAME_SIZE];
};

struct SocketBuffer
{
    char buffer[MAX_BUFFER_SIZE];
    int pos;
};

//////////////////////////////////////////////// Read & Write Methods ///////////////////////////////////////////////


/**
 *  Reads a string from keyboard input, with a maximum string size,
 *  and copy the resulting string to a specific buffer position.
 *
 *  @param buffer         Buffer in which the string is being copied on.
 *  @param bufferPosition The position of the buffer that the string is going to be copied on.
 *  @param maxStringSize  Max length of the string.
 *
 *  @return returns the resulting buffer position after copying the string on the buffer.
 */
int readString(char buffer[TAM_BUF], int bufferPosition, int maxLength) {
    char string[maxLength+1];
    int i;
    fgets(string, maxLength, stdin);
    for (i=0 ; string[i] ; i++) {
        if (string[i] == '\n') break;
        buffer[bufferPosition++] = string[i];
    }
    buffer[bufferPosition++] = '\0';
    return bufferPosition;
}

void clearBuffer(struct SocketBuffer *buff) {
    buff->pos = 0;
}

void bufferWrite(void *ptr, int size,struct SocketBuffer *buff) {
    memcpy(&buff->buffer[buff->pos],ptr,size);
    buff->pos+=size;
}

void writeInt(int value,struct SocketBuffer *buff) {
    bufferWrite(&value, 4,buff);
}

void sendPacket(struct SocketBuffer *buff, int socket) {
    ssize_t totalToSend, totalSent, sentBytes;
    struct SocketBuffer sendBuff;
    
    clearBuffer(&sendBuff);
    writeInt(buff->pos,&sendBuff);
    bufferWrite((void*)buff->buffer,buff->pos,&sendBuff);
    
    totalToSend = sendBuff.pos;
    totalSent = 0;
    
    while (totalSent < totalToSend) {
        sentBytes = send(socket, sendBuff.buffer + totalSent,(totalToSend-totalSent), 0);
        if (sentBytes < 0) {
            perror("Send()");
            return;
        }
        totalSent += sentBytes;
    }
}

void receivePacket(struct SocketBuffer *buff, int socket) {
    ssize_t bytesToReceive = 0, bytesReceived, totalBytesReceived = 0, sizeToReceive = 0;
    clearBuffer(buff);
    
    sizeToReceive = sizeof(int);
    
    //receives packet size
    while (totalBytesReceived < sizeToReceive) {
        bytesReceived = recv(socket, &bytesToReceive + totalBytesReceived, sizeToReceive - totalBytesReceived, 0);
        if (bytesReceived  == -1) {
            perror("Recv()");
            exit(6);
        }
        totalBytesReceived += bytesReceived;
    }
    
    //receives packet
    totalBytesReceived = 0;
    bytesReceived = 0;
    while (bytesReceived < bytesToReceive) {
        bytesReceived = recv(socket, buff->buffer + totalBytesReceived,bytesToReceive - totalBytesReceived, 0);
        if (bytesReceived  == -1) {
            perror("Recv()");
            exit(6);
        }
        totalBytesReceived += bytesReceived;
    }
}

//////////////////////////////////////////////// File Methods ///////////////////////////////////////////////

/**
 *  Persists a contact's information in file. If it already
 *  exists, replaces it. Otherwise, creates a new record.
 *
 *  @param contact The contact that is going to be saved.
 *
 *  @return Returns 1 if operation succeeds, otherwise 0.
 */
bool saveContact(Contact *contact) {
    
    bool operationSucceed = true, triedCreating = false;
    
    if (!contact) {
        return !operationSucceed;
    }
    
    char phone[PHONE_SIZE];
    int contactStringSize = PHONE_SIZE+INET_ADDRSTRLEN+PORT_SIZE; //11+16+5+2 commas + \n = [35];
    char tempSearch[contactStringSize];
    int line_num = 1, contact_exists = 0;
    
    strcpy(phone, contact->phone);
    
    FILE *contactsBook;
    
    do {
        //try to open the contacts book file
        contactsBook = fopen("contactsBook.txt", "r+");
        
        if (!contactsBook) { //unsuccess case
            // try to create the contacts book file
            if (!triedCreating) {
                triedCreating = true;
                fclose(fopen("contactsBook.txt", "w"));
            } else {
                printf("Error opening file.");
                return !operationSucceed;
            }
        } else {
            break; //check if this breaks from if or from loop
            //triedCreating = false; //if it breaks only from if, uncomment this line.
        }
    } while (triedCreating);
    
    while (fgets(tempSearch, contactStringSize, contactsBook) != NULL) {
        if ((strstr(tempSearch, phone)) != NULL) {
            printf("Found contact on contacts book line: %d\n", line_num);
            printf("Contact information: %s\n", tempSearch);
            contact_exists++;
        }
        line_num++;
    }
    
    if (contact_exists == 0) {
        printf("\nNo contact found with the given phone number. Adding it to the contacts book.\n");
//        return addContact(contact, database);
        
    } else {
        //to-do: implement contact replacement;
    }
    
    if (contactsBook) {
        fclose(contactsBook);
    }
    
    return operationSucceed;
}

//////////////////////////////////////////////// User Action Methods ///////////////////////////////////////////////

void addContact() {
    Contact *newContact = (Contact *)malloc(sizeof(Contact));
    
    printf("\nDigite o nome do contato: ");
    readString(newContact->name, 0, MAX_NAME_SIZE);
    printf("\nDigite o numero de celular deste contato: ");
    readString(newContact->phone, 0, PHONE_SIZE);
    
    saveContact(newContact);
}


void createGroup() {
    
}

void sendMessage() {
    
}

void sendImage() {
    
}

void disconnect() {
    
}

//////////////////////////////////////////////// Helper Methods ///////////////////////////////////////////////

int connectToServer(const char *hostnameParam, const char *portParam) {
    
    unsigned short port;
    struct hostent *hostname;
    struct sockaddr_in server;
    int returnSocket;
    
    //Obtains server IP address
    hostname = gethostbyname(hostnameParam);
    if (hostname == (struct hostent *)0) {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short)atoi(portParam);
    
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

void printMenu() {
    //improves readability
    printf("\n**********************************\n");
    printf("Opcoes:\n");
    printf("1 - Adicionar Contato\n");
    printf("2 - Criar Grupo de Broadcast\n");
    printf("3 - Enviar Mensagem\n");
    printf("4 - Enviar Foto\n");
    printf("5 - Sair\n");
    printf("**********************************\n");
}

//////////////////////////////////////////////// Main ///////////////////////////////////////////////

int main(int argc, const char * argv[]) {
    
    
    
    int socket = connectToServer(argv[1], argv[2]);
    
    
    bool connected = true;
    do {
        int userAction;
        do {
            printMenu();
            scanf("%d",&userAction);
        } while (userAction < 1 || userAction > 5);
        switch (userAction) {
            case userAction_AddContact: {
                addContact();
                break;
            }
            case userAction_CreateGroup: {
                createGroup();
                break;
            }
            case userAction_SendMessage: {
                sendMessage();
                break;
            }
            case userAction_SendImage: {
                sendImage();
                break;
            }
            case userAction_Disconnect: {
                connected = false;
                break;
            }
            default: {
                printf("\nAcao invalida.");
            }
        }
    } while (connected);
    
    close(socket);
    
    printf("Cliente terminou com sucesso.\n");
    return 0;
}