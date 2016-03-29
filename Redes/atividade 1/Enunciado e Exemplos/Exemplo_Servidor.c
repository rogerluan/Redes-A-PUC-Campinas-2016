#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Servidor UDP
 */
main()
{
   int sockint,s, namelen, client_address_size;
   struct sockaddr_in client, server;
   char buf[32];

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
   server.sin_family      = AF_INET;   /* Tipo do endereço             */
   server.sin_port        = 0;         /* Escolhe uma porta disponível */
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
   printf("Recebida a mensagem %s do endereço IP %s da porta %d\n",buf,inet_ntoa(client.sin_addr),ntohs(client.sin_port));

   /*
    * Fecha o socket.
    */
   close(s);
}
