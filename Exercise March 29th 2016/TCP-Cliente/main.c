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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // to get rid of close() warning

typedef struct {
    char userName[32];
    char text[32];
} Message;

//typedef struct {
//    Message messages[5];
//} MessageList;

void printMessages(Message message);

void printHomeMenu() {
    printf("Opcoes:\n\n1 - Cadastrar mensagem\n\n2 - Ler mensagens\n\n3 - Apagar mensagens\n\n4 - Sair da Aplicacao\n\n");
}

void sendMessage(int socket, char* str, int buflen) {

    Message newMessage;
    
    printf("Usuario: ");
    fgets(str,buflen,stdin);
    str[ strlen(str) - 1 ] = '\0';
    
    strcpy(newMessage.userName, str);
    
    printf("Mensagem: ");
    fgets(str,buflen,stdin);
    str[ strlen(str) - 1 ] = '\0';
    
    strcpy(newMessage.text, str);
    
    /* Envia a mensagem no buffer para o servidor */
    if (send(socket, &newMessage, sizeof(newMessage), 0) < 0) {
        perror("send()");
        exit(2);
    }
}

void receiveMessages(int socket) {
    
    Message newMessage;
    Message array[5];
    int msgCount = 0;
    
    long msgSize = recv(socket, array, sizeof(array), 0);
    if (msgSize < 0) {
        perror("recv()");
        exit(1);
    }
    
    for (int i = 0; i < 5; i++) {
        if (strcmp(array[i].userName,"\0") == 0) {
            break;
        }
        msgCount++;
    }
    
    printf("Mensagens cadastradas: %d\n\n",msgCount);
    
    for (int i = 0; i < msgCount; i++) {
        newMessage = array[i];
        printMessages(newMessage);
    }
}

void printMessages(Message message) {
    
    printf("Usuario: %s\nMessage: %s\n\n",message.userName,message.text);
//    
//    int i=0, count=0;
//    
//    if (recv(s, buf, buflen, 0) <0) {
//        perror("recv()");
//        exit(1);
//    }
//    
//    count = atoi(buf);
//    printf("Mensagens cadastradas: %d\n\n", count);
//    
//    for (i=0; i<count; i++) {
//        if(recv(s, buf, buflen, 0) <0)
//        {
//            perror("recv()");
//            exit(1);
//        }
//        
//        printf("Usuario: %s\t", buf);
//        fflush(stdout);
//        
//        if(recv(s, buf, buflen, 0) <0)
//        {
//            perror("recv()");
//            exit(1);
//        }
//        
//        printf("Mensagem: %s\n\n", buf);
//        fflush(stdout);
//    }
}

/*
 * Cliente TCP
 */
//int main(argc, argv)
//int argc;
//char **argv;
//{
int main(int argc, const char * argv[]) {
    unsigned short port;
    char sendbuf[32];
//    char recvbuf[32];
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
    
    /* Estabelece conex„o com o servidor */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect()");
        exit(4);
    }
    
    strcpy(sendbuf, "a");
    while(strcmp(sendbuf, "4") != 0) {
        
        printHomeMenu();
        fgets(sendbuf,sizeof(sendbuf) - 1, stdin);
        sendbuf[ strlen(sendbuf) - 1 ] = '\0';
        
        /* Envia a mensagem no buffer para o servidor */
        if (send(sock, sendbuf, sizeof(sendbuf), 0) < 0) {
            perror("send()");
            exit(2);
        }
        if (strcmp(sendbuf, "1") == 0) {
            sendMessage(sock, sendbuf, sizeof(sendbuf));
        } else if (strcmp(sendbuf, "2") == 0) {
            receiveMessages(sock);
//            printAllMessagens(s, sendbuf, sizeof(sendbuf));
        } else if (strcmp(sendbuf, "3") == 0) {
            /* implement message deletion function */
        }
    }
    
    /* Fecha o socket */
    close(sock);
    
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}
