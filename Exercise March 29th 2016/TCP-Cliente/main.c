//
//  main.c
//  TCP-Cliente
//
//  Created by Roger Luan on 3/22/16.
//  Copyright © 2016 Roger Oba. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
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
} SendMessage;

void printMessage(Message message);

void printHomeMenu() {
    printf("Opcoes:\n\n1 - Cadastrar mensagem\n\n2 - Ler mensagens\n\n3 - Apagar mensagens\n\n4 - Sair da Aplicacao\n\n");
}

Message createMessage() {

    Message newMessage;
    
    printf("Usuario: ");

    fgets(newMessage.userName, 32, stdin);
    newMessage.userName[ strlen(newMessage.userName) - 1 ] = '\0';
    
    printf("\nMensagem: ");
    fgets(newMessage.userName, 32, stdin);
    newMessage.userName[ strlen(newMessage.userName) - 1 ] = '\0';
    
    return newMessage;
}

void receiveMessages(Message data[], int size) {
    
    printf("Mensagens cadastradas: %d\n\n", size);
    
    for (int i = 0; i < size; i++) {
        
        printMessage(data[i]);
    }
}

void printMessage(Message message) {
    printf("Usuario: %s\nMessage: %s\n\n", message.userName, message.text);
}

/*
 * Cliente TCP
 */
int main(int argc, const char * argv[]) {
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;
    int sock;
    
    /*
     * O primeiro argumento (argv[1]) é o hostname do servidor.
     * O segundo argumento (argv[2]) é a porta do servidor.
     */
    if (argc != 3) {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }
    
    /*
     * Obtendo o endereço IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0) {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);
    
    /*
     * Define o endereço IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned int *)hostnm->h_addr);
    
    /*
     * Cria um socket TCP (stream)
     */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
        exit(3);
    }
    
    /* Estabelece conexão com o servidor */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect()");
        exit(4);
    }
    
    SendMessage sendMessage;
    sendMessage.cod = -1;
    
    while(sendMessage.cod != 0) {
        
        printHomeMenu();
        
        char option[32];
        fgets(option, 32, stdin);
        sendMessage.cod = (int)option[0]-48;
        
        if (sendMessage.cod == 1) {
            
            sendMessage.dataParam[0] = createMessage();
            
            /* Envia a mensagem no buffer para o servidor */
            if (send(sock, &sendMessage, sizeof(sendMessage), 0) < 0) {
                perror("send()");
                exit(2);
            }
            
            Response responseMessage;
            if(recv(sock, &responseMessage, sizeof(responseMessage), 0) < 0) {
                perror("recv()");
                exit(2);
            } else {
                
                if (responseMessage.error == 0) {
                    printf("Mesagem Cadastrada com sucesso.\n");
                } else {
                    printf("Erro %d - %s\n", responseMessage.error, responseMessage.messageError);
                }
            }
        } else if (sendMessage.cod == 2) {
            
            /* Envia a mensagem no buffer para o servidor */
            if (send(sock, &sendMessage, sizeof(sendMessage), 0) < 0) {
                perror("send()");
                exit(2);
            }
            
            Response responseMessage;
            if(recv(sock, &responseMessage, sizeof(responseMessage), 0) < 0) {
                perror("recv()");
                exit(2);
            }
            
            receiveMessages(responseMessage.data, responseMessage.sizeData);
        } else if (sendMessage.cod == 3) {
            
            
            char *user;
            printf("Usuario: ");
            fgets(user, 32, stdin);
            user[ strlen(user) - 1 ] = '\0';
            
            strcpy(sendMessage.dataParam[0].userName, user);
            
            /* Envia a mensagem no buffer para o servidor */
            if (send(sock, &sendMessage, sizeof(sendMessage), 0) < 0) {
                perror("send()");
                exit(2);
            }
            
            Response responseMessage;
            if(recv(sock, &responseMessage, sizeof(responseMessage), 0) < 0) {
                perror("recv()");
                exit(2);
            } else {
                
                if (responseMessage.error == 0) {
                    printf("Mesagem excluida com sucesso.\n");
                    printMessage(responseMessage.data[0]);
                } else {
                    printf("Erro %d - %s\n", responseMessage.error, responseMessage.messageError);
                }
            }
        }
        
    }
    
    /* Fecha o socket */
    close(sock);
    
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
