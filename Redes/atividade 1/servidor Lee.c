#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/*
 * Servidor UDP
 */

char **argv;


main(argv)

{
	int sockint,s, namelen, client_address_size;
	struct sockaddr_in client, server;
   	
	char nome[10];
   	char mensagem[50];
   	int i = 0, j = 0, op = 0;                                                              
   	char nome1[10] = "";
   	char nome2[10] = "";
   	char nome3[10] = "";
   	char nome4[10] = "";
   	char nome5[10] = "";
   	char mensagem1[50] = "";
   	char mensagem2[50] = "";
   	char mensagem3[50] = "";
   	char mensagem4[50] = "";
   	char mensagem5[50] = "";
	char opcao[4] = "";	
	
   	/*
    	* Cria um socket UDP (dgram). 
    	*/
   	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   	{
       		perror("socket()");
       		exit(1);
   	}

   /*
    * Define a qual endere�o IP e porta o servidor estar� ligado.
    * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os endere�os IP
    */
   	server.sin_family      = AF_INET;   /* Tipo do endere�o             */
   	server.sin_port        = 0;         /* Escolhe uma porta dispon�vel */
   	server.sin_addr.s_addr = INADDR_ANY;/* Endere�o IP do servidor      */
	
   /*
    * Liga o servidor � porta definida anteriormente.
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
   	printf("Porta utilizada � %d\n", ntohs(server.sin_port));
   /*

    * Recebe uma mensagem do cliente.
    * O endere�o do cliente ser� armazenado em "client".
    */

  do
    {
   	client_address_size = sizeof(client);
	
	if(recvfrom(s, mensagem, sizeof(mensagem), 0, (struct sockaddr *) &client, &client_address_size) <0)
   	{
       		perror("recvfrom()");
       		exit(1);
   	}
	
   	if(recvfrom(s, nome, sizeof(nome), 0, (struct sockaddr *) &client, &client_address_size) <0)
   	{
       		perror("recvfrom()");
       		exit(1);
   	}

   	
	if(recvfrom(s, opcao, sizeof(opcao), 0, (struct sockaddr *) &client, &client_address_size) <0)
   	{
       		perror("recvfrom()");
       		exit(1);
   	}
	
	if(strcmp(opcao,"um") == 0)
		op = 1;
	if(strcmp(opcao,"dois") == 0)
		op = 2;
	if(strcmp(opcao,"tres") == 0)
		op = 3;

   
   /*
    * Imprime a mensagem recebida, o endere�o IP do cliente
    * e a porta do cliente 
    
   printf("Nome = %s \n", nome);
   printf("Mensagem = %s \n", mensagem);
   */
		
	

	//printf("%d",op);

	if(op == 1)
  	{
 		for(i = 0; i < 5; i++ )
  		 {
			
			switch(i)
			{	
				case 1:
	   		        if(strcmp(nome1,"")==0)
				{	
							
					strcpy(nome1,nome);
					strcpy(mensagem1,mensagem);
					i = 6;
					break;			
				}		

				case 2:
                		if(strcmp(nome2,"")==0)
				{			
					strcpy(nome2,nome);
					strcpy(mensagem2,mensagem);
					i = 6;
					break;			
				}
		
				case 3:
				if(strcmp(nome3,"")==0)
				{			
					strcpy(nome3,nome);
					strcpy(mensagem3,mensagem);
					i = 6;
					break;			
				}
		
		

				case 4:
				if(strcmp(nome4,"")==0)
				{			
					strcpy(nome4,nome);
					strcpy(mensagem4,mensagem);
					i = 6;			
					break;
				}

				case 5:
				if(strcmp(nome5,"")==0)
				{			
					strcpy(nome5,nome);
					strcpy(mensagem5,mensagem);
					i = 6;			
				break;
				}
			}
   		}
	}
	
        else if(op == 2)
	{        
		if (sendto(s, mensagem1, (strlen(mensagem1)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, mensagem2, (strlen(mensagem2)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, mensagem3, (strlen(mensagem3)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}
		
		if (sendto(s, mensagem4, (strlen(mensagem4)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, mensagem5, (strlen(mensagem5)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, nome1, (strlen(nome1)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)

	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, nome2, (strlen(nome2)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)

	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, nome3, (strlen(nome3)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)

	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}

		if (sendto(s, nome4, (strlen(nome4)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)

	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}
	
		if (sendto(s, nome5, (strlen(nome5)+1), 0, (struct sockaddr *)&client, sizeof(client)) < 0)

	   	{
	       		perror("sendto()");
	       		exit(2);
	   	}
		
        	printf("\nNome1 = %s\n",nome1);
        	printf("Mensagem1 = %s\n\n",mensagem1);
        	printf("Nome2 = %s\n",nome2);
        	printf("Mensagem2 = %s\n\n",mensagem2);
        	printf("Nome3 = %s\n",nome3);   
        	printf("Mensagem3 = %s\n\n",mensagem3);
        	printf("Nome4 = %s\n",nome4);
        	printf("Mensagem4 = %s\n\n",mensagem4);
        	printf("Nome5 = %s\n",nome5);
        	printf("Mensagem5 = %s\n\n",mensagem5);

	}
	
  }while(op != 3);

   	//inet_ntoa(client.sin_addr),ntohs(client.sin_port));

   /*
    * Fecha o socket.
    */
   	close(s);
	exit(1);
}


