#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
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

Message reiciveMessage(int s, char *buf, int buflen) {
    
    Message newMessage;
    
    int msgSize = recv(s, buf, buflen, 0);
    if(msgSize <0)
    {
        perror("recv()");
        exit(1);
    }
    printf("reiciveMessage 1 %s %d\n", buf, msgSize);
    fflush(stdout);
    
    strcpy(newMessage.userName, buf);
    fflush(stdout);
    
    if(recv(s, buf, buflen, 0) <0)
    {
        perror("recv()");
        exit(1);
    }
    
    printf("reiciveMessage 2 %s\n", buf);
    fflush(stdout);

    strcpy(newMessage.text, buf);
    fflush(stdout);
    return newMessage;
}

void sendAllMessagens(Message data[], int size, int s, char *buf, int buflen){
    
    int i = 0, count = 0;
    
    count = countData(data, size);
    
    sprintf(buf, "%d", count);
    
    if (send(s, buf, buflen+1, 0) < 0)
    {
        perror("send()");
        exit(2);
    }
    
    printf("%s\n", buf);
    fflush(stdout);
    
    for (i=0; i<count; i++) {
        
        strcpy(buf, data[i].userName);
        if (send(s, buf, buflen+1, 0) < 0)
        {
            perror("sendto()");
            exit(2);
        }
        
        printf("%s\n", buf);
        fflush(stdout);
        
        strcpy(buf, data[i].text);
        if (send(s, buf, buflen+1, 0) < 0)
        {
            perror("send()");
            exit(2);
        }
        
        printf("%s\n", buf);
        fflush(stdout);

    }
}

/*
 * Servidor TCP
 */
int main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;       
    char sendbuf[32];
    char recvbuf[32];
    struct sockaddr_in client; 
    struct sockaddr_in server; 
    int s;                     /* Socket para aceitar conexões       */
    int ns;                    /* Socket conectado ao cliente        */
    socklen_t namelen;
    
    Message data[SIZE];
    cleanArrayData(data, SIZE);

    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
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

    /*
     * Liga o servidor à porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
       perror("Bind()");
       exit(3);
    }

    /*
     * Prepara o socket para aguardar por conexões e
     * cria uma fila de conexões pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }

    /*
     * Aceita uma conexão e cria um novo socket através do qual
     * ocorrerá a comunicação com o cliente.
     */
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }
    
    
    strcpy(recvbuf, "a");
    
    while(strcmp(recvbuf, "4") != 0) {
        
        if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        printf("recv Main    %s\n", recvbuf);
        fflush(stdout);
        
        if(strcmp(recvbuf, "1") == 0) {
            
            Message m = reiciveMessage(ns, recvbuf, sizeof(recvbuf));
            
            insetMessage( m, data, SIZE);
            
        } else if(strcmp(recvbuf, "2") == 0) {
            
            sendAllMessagens(data, SIZE, ns, recvbuf, sizeof(recvbuf));
        }
    }

    /* Fecha o socket conectado ao cliente */
    close(ns);

    /* Fecha o socket aguardando por conexões */
    close(s);

    printf("Servidor terminou com sucesso.\n");
    exit(0);
}


