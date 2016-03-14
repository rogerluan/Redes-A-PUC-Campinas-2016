#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Servidor UDP
 */
main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;
    int sockint,s, namelen, client_address_size;
    struct sockaddr_in client, server;
    char buf[32];
    
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
    printf("Porta utilizada é %d\n", ntohs(server.sin_port));
    
    /*
     * Recebe uma mensagem do cliente.
     * O endereço do cliente será armazenado em "client".
     */
    client_address_size = sizeof(client);
    strcpy(buf, "a");
    
    while(strcmp(buf, "3") != 0) {
        if(recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &client,
                    &client_address_size) <0)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        /*
         * Imprime a mensagem recebida, o endereço IP do cliente
         * e a porta do cliente
         */
        printf("%s\n", buf);
    }
    
    /*
     * Fecha o socket.
     */
    close(s);
}
