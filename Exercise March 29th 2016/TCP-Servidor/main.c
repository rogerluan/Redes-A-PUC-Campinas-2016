//
//  main.c
//  TCP-Servidor
//
//  Created by Roger Luan on 3/22/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define SIZE 5

typedef struct {
    
    char userName[32];
    char text[32];
} Message;

typedef struct {
    
    char *messageError;
    int error;
    int sizeData;
    Message data[SIZE];
} Response;

typedef struct {
    
    int cod;
    int sizeData;
    Message dataParam[SIZE];
} ReceiveMessage;

int countData(Message data[], int size){
    int i = 0;
    for (i=0; i<size; i++) {
        if (strcmp(data[i].userName, "") == 0) {
            return i;
        }
    }
    return size;
}

void cleanArrayData(Message data[], int size){
    for (int i = 0; i < size; i++) {
        strcpy(data[i].userName,"\0");
        strcpy(data[i].text,"\0");
    }
}

void deleteWithName(char* name, Message data[], int size) {
    
    int i=0, index=0;
    
    for(i=0; i<size; i++) {
        if (strcmp(data[i].userName, name) == 0) {
            index = i;
            break;
        }
    }
    
    for (i = index; i<size -1; i++) {
        data[i] = data[i+1];
    }
}

void insertMessage(Message data[], Message message) {
    
    for (int i = 0; i < SIZE; i++) {
        if (strcmp(data[i].userName, "\0") == 0) {
            data[i] = message;
            break;
        }
    }
}

Message receiveMessage(int socket) {
    
    Message newMessage;
    
    long msgSize = recv(socket, &newMessage, sizeof(newMessage), 0);
    if (msgSize < 0) {
        perror("recv()");
        exit(1);
    }
    
    printf("Received username: %s\n", newMessage.userName);
    printf("Received message: %s\n", newMessage.text);
    
    return newMessage;
}

void sendAllMessages(int sock, Message data[]) {
    
    /* Envia a mensagem para o cliente */
    if (send(sock, data, sizeof(&data), 0) < 0) {
        perror("send()");
        exit(2);
    } else {
        printf("Successfully sent all messages.");
    }
}

void runProgram(int newSocket, Message data[], int size) {
    
    char recvbuf[32];
    
    while(strcmp(recvbuf, "4") != 0) {
        long recvReturn = recv(newSocket, recvbuf, sizeof(recvbuf), 0);
        if (recvReturn == -1 || recvReturn == 0) {
            break;
        }
        
        printf("[RunProgram] Received Action: %s\n", recvbuf);
        fflush(stdout);
        
        if (strcmp(recvbuf, "1") == 0) {
            
            Message messageReceived = receiveMessage(newSocket);
            insertMessage(data, messageReceived);
            //            insertMessageLegacy(messageReceived,data,SIZE);
            
        } else if(strcmp(recvbuf, "2") == 0) {
            //            sendAllMessages(newSocket,data);
            
            /* Envia a mensagem para o cliente */
            if (send(newSocket, data, sizeof(&data), 0) < 0) {
                perror("send()");
                exit(2);
            } else {
                printf("Successfully sent all messages.");
            }
            
        } else if(strcmp(recvbuf, "3") == 0) {
            /**
             * implement message deleting
             */
        } else {
            /**
             * implementar envio de mensagem de erro caso a entrada seja
             * diferente de um numero entre 1 e 4
             */
        }
    }
}

/*
 * Servidor TCP
 */
int main(int argc, const char * argv[]) {
    unsigned short port;
    Message data[SIZE];
    
    struct sockaddr_in client;
    struct sockaddr_in server;
    int sock;                     /* Socket para aceitar conexões       */
    int newSocket;                /* Socket conectado ao cliente        */
    socklen_t namelen;
    pid_t pid;
    
    cleanArrayData(data, SIZE);
    
    /*
     * O primeiro argumento (argv[1]) È a porta
     * onde o servidor aguardar· por conexıes
     */
    if (argc != 2) {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }
    
    port = (unsigned short) atoi(argv[1]);
    
    /*
     * Cria um socket TCP (stream) para aguardar conexıes
     */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
        exit(2);
    }
    
    /*
     * Define a qual endereço IP e porta o servidor estará ligado.
     * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
     * os endereÁos IP
     */
    server.sin_family = AF_INET;
    server.sin_port   = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    
    /*
     * Liga o servidor ‡ porta definida anteriormente.
     */
    
    int on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind()");
        exit(3);
    }
    
    /*
     * Prepara o socket para aguardar por conexıes e
     * cria uma fila de conexıes pendentes.
     */
    if (listen(sock, 1) != 0) {
        perror("Listen()");
        exit(4);
    }
    
    
    while(1) {
        /*
         * Aceita uma conexão e cria um novo socket através do qual
         * ocorrerá a comunicação com o cliente.
         */
        namelen = sizeof(client);
        if ((newSocket = accept(sock, (struct sockaddr *)&client, &namelen)) == -1) {
            perror("Accept()");
            exit(5);
        }
        
        /* Cria-se um novo processo */
        if ((pid = fork()) < 0) {
            /* O fork retornou algum erro na hora de ser executado */
            perror("Fork()");
            exit(7);
            
        } else if (pid == 0) {
            /*
             * Processo filho
             */
            
            
            /* Roda a logica do programa */
//            runProgram(newSocket,data,SIZE);
            
            /* O código abaixo é equivalente ao método runProgram
             * >>>> Perguntar ao Edmar como passar o array como parâmetro
             * para ser usado posteriormente nas chamadas send() <<<<<<
             */
//            char recvbuf[32];
            ReceiveMessage receive;
            receive.cod = -1;
            
            while(receive.cod != 0) {
                
                long recvReturn = recv(newSocket, &receive, sizeof(receive), 0);
                if (recvReturn == -1 || recvReturn == 0) {
                    break;
                }
                
                printf("[RunProgram] Received Action: %d\n", receive.cod);
                fflush(stdout);
                
                if (receive.cod == 1) {
                    
                    insertMessage(data, receive.dataParam[0]);
                    
                    Response responseMessage;
                    responseMessage.error = 0;
                    
                    if (send(newSocket, &responseMessage, sizeof(responseMessage), 0) < 0) {
                        perror("send()");
                        exit(2);
                    }
                    
                } else if(receive.cod == 2) {
                    
                    Response responseMessage;
                    responseMessage.error = 0;
                    
                    int i = 0;
                    
                    for(i = 0; i < SIZE; i++){
                        
                        if (strcmp(data[i].userName, "\0") == 0) {
                            break;
                        } else {
                            
                            responseMessage.data[i] = data[i];
                        }
                    }
                    
                    responseMessage.sizeData = i;
                    
                    if (send(newSocket, &responseMessage, sizeof(responseMessage), 0) < 0) {
                        perror("send()");
                        exit(2);
                    }
                    
                } else if(receive.cod == 3) {
                    /**
                     * implement message deleting
                     */
                    
                    deleteWithName(receive.dataParam[0].userName, data, SIZE);
                    
                    Response responseMessage;
                    responseMessage.error = 0;
                    
                    if (send(newSocket, &responseMessage, sizeof(responseMessage), 0) < 0) {
                        perror("send()");
                        exit(2);
                    }
                    
                } else {
                    /**
                     * implementar envio de mensagem de erro caso a entrada seja
                     * diferente de um numero entre 1 e 4
                     */
                }
            }
            
            /**
             * implementar envio de mensagem de finalização
             */
            
            /* Fecha o socket conectado ao cliente */
            close(newSocket);
        } else {
            /*
             * Processo pai
             */
            
            printf("Processo filho criado: %d\n", pid);
            
            /* Fecha o socket conectado ao cliente */
            close(newSocket);
        }
    }
}



