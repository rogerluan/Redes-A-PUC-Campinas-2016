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
    char userName[10];
    char text[50];
} Message;

void printHomeMenu();

/*
 * Cliente TCP
 */
int main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;
    char sendbuf[12];
    char recvbuf[12];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;
    
    /*
     * O primeiro argumento (argv[1]) Ž o hostname do servidor.
     * O segundo argumento (argv[2]) Ž a porta do servidor.
     */
    if (argc != 3) {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }
    
    /*
     * Obtendo o endereo IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0) {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);
    
    /*
     * Define o endereo IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
    
    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket()");
        exit(3);
    }
    
    /* Estabelece conexão com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect()");
        exit(4);
    }
    
    strcpy(sendbuf, "0");
    
    while(strcmp(sendbuf, "4") != 0) {
        
        printHomeMenu();
        
        scanf("%s", &sendbuf);
        
        /* Envia a mensagem no buffer de envio para o servidor */
        if (send(s, sendbuf, strlen(sendbuf)+1, 0) < 0) {
            perror("Send()");
            exit(5);
        }
        printf("Mensagem enviada ao servidor: %s\n", sendbuf);
        
        /* Recebe a mensagem do servidor no buffer de recep‹o */
        if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0) {
            perror("Recv()");
            exit(6);
        }
        printf("Mensagem recebida do servidor: %s\n", recvbuf);
    }
    
    
    /* Fecha o socket */
    close(s);
    
    printf("Cliente terminou com sucesso.\n");
    exit(0);
}

void printHomeMenu(){
    printf("Opcoes:\n\n1 - Cadastrar mensagem\n\n2 - Ler mensagens\n\n3 - Apagar mensagens\n\n4 - Sair da Aplicacao\n\n");
}



