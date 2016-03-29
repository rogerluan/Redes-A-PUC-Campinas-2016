#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void funcClient(struct sockaddr_in server, int s)
{
   	char texto[50], usuario[10], confirm;
   	int opt, qtdMsgs, countMsgs, client_address_size, server_address_size;
   	struct sockaddr_in client;

	//Imprime o Menu ao usuario
	printf("Opcoes:\n
		1 - Cadastrar Mensagem\n
		2 - Ler Mensagens Salvas\n
		3 - Sair da Aplicacao\n
		Opcao Desejada: ");
	scanf("%d", &opt);

	switch(opt)
	{
		case(1): // 1 = Enviar Mensagem
		{
			//Solicita que o Usuario digite seu nome e sua mensagem
			printf("Digite seu nome (Maximo de 10 caracteres): ");
			scanf("%s", usuario); 

			printf("Digite a mensagem desejada (Maximo de 50 caracteres): ");
			scanf("%s", texto); 


			// Envia o nome do usuario e sua mensagem para o servidor
			if (sendto(s, usuario, (strlen(usuario)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
			{
				perror("sendto(usuario)");
				exit(2);
			}
			if (sendto(s, texto, (strlen(texto)+1), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
			{
				perror("sendto(texto)");
				exit(2);
			}
			
			break;
		}
		case(2): //2 = Imprimir todas as mensagens
		{
	            sendto(s, "ReturnMsg", (strlen("ReturnMsg")+1), 0, (struct sockaddr *)&server, sizeof(server));
			
	            recvfrom(s, &qtdMsgs, sizeof(qtdMsgs), 0, (struct sockaddr *) &server, &server_address_size); //recebe o numero de mensagens salvas

			for(countMsgs = 0; countMsgs < qtdMsgs; countMsgs++)
			{
				//Recebe os nomes e mensagens salvas no servidor e imprime todas elas
				recvfrom(s, usuario, sizeof(usuario), 0, (struct sockaddr *) &server, &server_address_size);
				recvfrom(s, texto, sizeof(texto), 0, (struct sockaddr *) &server, &server_address_size);
				
				printf("\nMensagem %d\n", countMsgs+1);
				printf("Usuario: %s\n", usuario);
				printf("Texto: %s\n", texto);
			}
			
			break;
		}
		case(3): //3 = Sair
		{
			exit(1);
			break;
		}
	}
		
	funcClient(server, s);
}

// Cliente UDP
int main(int argc, char *argv[])
{
  	int porta, s;
  	struct sockaddr_in server, client;

   //Verifica se os parametros para a funcao foram usados corretamente
   if(argc != 3)
   {
      printf("Use: %s enderecoIP porta\n",argv[0]);
      exit(1);
   }
   
	porta = htons(atoi(argv[2]));
	  
	// Define o endereço IP e a porta do servidor
	server.sin_family      = PF_INET;              // Tipo do endereço
	server.sin_port        = porta;                // porta do servidor
	server.sin_addr.s_addr = inet_addr(argv[1]);   // Endereço IP do servidor 
	 
	// Define o endereço IP e a porta do cliente
	client.sin_family      = PF_INET;                // Tipo do cliente
	client.sin_port        = porta+1;                // porta do cliente
	client.sin_addr.s_addr = inet_addr(argv[1]);     // Endereço IP do cliente 

	//Define o Socket UDP (DGRAM)
	if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
   	{
     	perror("socket()");
       	exit;
   	}
  
   	//Liga o servidor à porta definida anteriormente.
   	if (bind(s, (struct sockaddr *)&client, sizeof(client)) < 0)
   	{
	   	perror("bind()");
	   	exit(1);
   	} 
      
  	funcClient(server, s);
      
   	// Fecha o socket
   	close(s);
	return 0;
}
