#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 5

typedef struct {
    char userName[32];
    char text[32];
} Message;

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
    
    int i=0;
    
    for(i=0; i<size; i++) {
        strcpy(data[i].userName,"");
        strcpy(data[i].text,"");
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

void insetMessage(Message message, Message data[], int size) {
    
    int i = size - 1;
    
    for (i = size - 1; i>0; i--) {
        data[i] = data[i-1];
    }
    
    data[0] = message;
}

Message reiciveMessage(int s, char *buf, struct sockaddr *from, socklen_t *fromlen) {
    
    Message newMessage;
    
    if(recvfrom(s, buf, 31*sizeof(char), 0, from, fromlen) <0)
    {
        perror("recvfrom()");
        exit(1);
    }
    
    strcpy(newMessage.userName, buf);
    fflush(stdout);
    
    if(recvfrom(s, buf, 31*sizeof(char), 0, from, fromlen) <0)
    {
        perror("recvfrom()");
        exit(1);
    }
    
    strcpy(newMessage.text, buf);
    fflush(stdout);
    return newMessage;
}

void sendAllMessagens(Message data[], int size, int s, char *buf, struct sockaddr *from, socklen_t fromlen){

    int i = 0, count = 0;
    
    count = countData(data, size);
    
    sprintf(buf, "%d", count);
    
    if (sendto(s, buf, 31*sizeof(char)+1, 0, from, fromlen) < 0)
    {
        perror("sendto()");
        exit(2);
    }
    
    for (i=0; i<count; i++) {
        
        strcpy(buf, data[i].userName);
        if (sendto(s, buf, 31*sizeof(char)+1, 0, from, fromlen) < 0)
        {
            perror("sendto()");
            exit(2);
        }
        
        strcpy(buf, data[i].text);
        if (sendto(s, buf, 31*sizeof(char)+1, 0, from, fromlen) < 0)
        {
            perror("sendto()");
            exit(2);
        }
    }
}

/*
 * Servidor UDP
 */
int main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;
    int sockint,s;
    socklen_t namelen, client_address_size;
    struct sockaddr_in client, server;
    char buf[32];
    
    Message data[SIZE];
    
    cleanArrayData(data, SIZE);
    
    /*
     * O primeiro argumento (argv[1]) é o endereço IP do servidor.
     * O segundo argumento (argv[2]) é a porta do servidor.
     */
    if(argc != 2)
    {
        printf("Use: %s porta\n", argv[0]);
        exit(1);
    }
    
    /*
     * Cria um socket UDP (dgram).
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }
    
    /*
     * Define a qual endereço IP e porta o servidor estará ligado.
     * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
     * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
     * os endereços IP
     */
    port = htons(atoi(argv[1]));
    
    server.sin_family      = AF_INET;   /* Tipo do endereço             */
    server.sin_port        = port;   /* Porta escolhida pelo usu‡rio */
    server.sin_addr.s_addr = INADDR_ANY;/* Endereço IP do servidor      */
    
    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind()");
        exit(1);
    }
    
    /* Consulta qual porta foi utilizada. */
    namelen = sizeof(server);
    if (getsockname(s, (struct sockaddr *) &server, &namelen) < 0)
    {
        perror("getsockname()");
        exit(1);
    }
    
    /* Imprime qual porta foi utilizada. */
    printf("Porta utilizada eh %d\n", ntohs(server.sin_port));
    
    /*
     * Recebe uma mensagem do cliente.
     * O endereço do cliente será armazenado em "client".
     */
    client_address_size = sizeof(client);
    strcpy(buf, "a");

    while(strcmp(buf, "3") != 0) {

        if(recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client,
                    &client_address_size) < 0)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        if(strcmp(buf, "1") == 0) {
            
            Message m = reiciveMessage(s, buf, (struct sockaddr *) &client,
                                       &client_address_size);
            
            insetMessage( m, data, SIZE);
            
        } else if(strcmp(buf, "2") == 0) {
        
            sendAllMessagens(data, SIZE, s, buf, (struct sockaddr *) &client,
                             client_address_size);
            
        }
        
        /*
         * Imprime a mensagem recebida, o endereço IP do cliente
         * e a porta do cliente
         */
    }
    
    /*
     * Fecha o socket.
     */
    close(s);
}
