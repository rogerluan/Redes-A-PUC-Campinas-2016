#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <netdb.h>

/*
 * Cliente UDP
 */
main(argc, argv)
int argc;
char **argv;
{
    
    
    int s;
    unsigned short port;
    struct sockaddr_in server;
    struct hostent *hostnm;
    char buf[32];
    
    
    /*
     * O primeiro argumento (argv[1]) � o endere�o IP do servidor.
     * O segundo argumento (argv[2]) � a porta do servidor.
     */
    if(argc != 3)
    {
        printf("Use: %s enderecoIP porta\n",argv[0]);
        exit(1);
    }
    
    
    /*
     * Obtendo o endere�o IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    
    //relatar isso (nao funciona muito bem crossplatform devido a diferenca entre little endian e big endian)
//    port = (unsigned short) atoi(argv[2]);

    port = htons(atoi(argv[2]));
    
    /*
     * Cria um socket UDP (dgram).
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }
    
    /* Define o endere�o IP e a porta do servidor */
    server.sin_family      = AF_INET;            /* Tipo do endere�o         */
    server.sin_port        = port;               /* Porta do servidor        */
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr); /* Endere�o IP do servidor  */
    
    printf("IP utilizado eh: %s\n", inet_ntoa(server.sin_addr));
    
    strcpy(buf, "Hello");
    
    /* Envia a mensagem no buffer para o servidor */
    if (sendto(s, buf, (strlen(buf)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("sendto()");
        exit(2);
    }
    
    /* Fecha o socket */
    close(s);
}
