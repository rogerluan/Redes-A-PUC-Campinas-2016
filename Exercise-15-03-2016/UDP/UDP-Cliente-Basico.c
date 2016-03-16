#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <netdb.h>

typedef struct {
    char userName[32];
    char text[32];
} Message;

void printHomeMenu(){
    
    printf("Opcoes:\n\n1 - Cadastrar mensagem\n\n2 - Ler mensagens\n\n3 - Sair da Aplicacao\n\n");
}

void sendMessage(int s, char* buf, int buflen, struct sockaddr *from, socklen_t fromlen){
 
    printf("Usuario: ");
    fgets(buf,buflen,stdin);
    buf[ strlen(buf) - 1 ] = '\0';
    /* Envia a mensagem no buffer para o servidor */
    if (sendto(s, buf, buflen, 0, from, fromlen) < 0)
    {
        perror("sendto()");
        exit(2);
    }
    
    printf("Mensagem: ");
    fgets(buf,buflen,stdin);
    buf[ strlen(buf) - 1 ] = '\0';
    
    /* Envia a mensagem no buffer para o servidor */
    if (sendto(s, buf, buflen+1, 0, from, fromlen) < 0)
    {
        perror("sendto()");
        exit(2);
    }
}

void printAllMessagens(int s, void *buf, int buflen, struct sockaddr *from, socklen_t *fromlen) {
    
    int i=0, count=0;
    
    if(recvfrom(s, buf, buflen, 0, from, fromlen) <0)
    {
        perror("recvfrom()");
        exit(1);
    }
    
    count = atoi(buf);
    printf("Mensagens cadastradas: %d\n\n", count);
    
    for (i=0; i<count; i++) {
        if(recvfrom(s, buf, buflen, 0, from, fromlen) <0)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        printf("Usuario: %s\t", buf);
        fflush(stdout);
        
        if(recvfrom(s, buf, buflen, 0, from, fromlen) <0)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        printf("Mensagem: %s\n\n", buf);
        fflush(stdout);
    }
}

/*
 * Cliente UDP
 */
int main(argc, argv)
int argc;
char **argv;
{
    int s;
    socklen_t server_address_size;
    unsigned short port;
    struct sockaddr_in server;
    struct hostent *hostnm;
    char buf[32];
    
    /*
     * O primeiro argumento (argv[1]) é o endereço IP do servidor.
     * O segundo argumento (argv[2]) é a porta do servidor.
     */
    if(argc != 3)
    {
        printf("Use: %s enderecoIP porta\n",argv[0]);
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
    
    port = htons(atoi(argv[2]));
    
    /*
     * Cria um socket UDP (dgram).
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }
    
    /* Define o endereço IP e a porta do servidor */
    server.sin_family      = AF_INET;            /* Tipo do endereço         */
    server.sin_port        = port;               /* Porta do servidor        */
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr); /* Endereço IP do servidor  */
    
    server_address_size = sizeof(server);
    
    strcpy(buf, "a");
    while(strcmp(buf, "3") != 0) {
        
        printHomeMenu();
        fgets(buf,31 * sizeof(char),stdin);
        buf[ strlen(buf) - 1 ] = '\0';
        
        /* Envia a mensagem no buffer para o servidor */
        if (sendto(s, buf, sizeof(buf)+1, 0, (struct sockaddr *)&server, server_address_size) < 0)
        {
            perror("sendto()");
            exit(2);
        }
        
        if (strcmp(buf, "1") == 0) {
            
            sendMessage(s, buf, sizeof(buf), (struct sockaddr *) &server, server_address_size);
            
        } else if (strcmp(buf, "2") == 0) {
            
            printAllMessagens(s, buf, sizeof(buf), (struct sockaddr *) &server, &server_address_size);
        }
    }
    /* Fecha o socket */
    close(s);
}


