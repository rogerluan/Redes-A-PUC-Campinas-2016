#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

//Prototipo dos Procedimentos
void menu(int s);

//Cliente TCP 
int main(int argc, char *argv[])
{
    unsigned short port;              
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s;                 
    
    
    //Verifica se o programa foi executado corretamente
    if (argc != 3)
    {
        printf("Use: %s hostname porta\n", argv[0]);
        exit(1);
    }

    
    //Obtendo o endereço IP do servidor     
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

    
    //Define o endereço IP e a porta do servidor
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    //Cria um socket TCP (stream)
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    // Estabelece conexão com o servidor 
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }

    menu(s);

    // Fecha o socket 
    close(s);
   return (0);
}

// Imprime o Menu inicial, faz a leitura e salva informacoes a serem enviadas
void menu(int s)
{
  int opcao = 0, count, confirmacao;  
  struct mensagens
  {
	  char usuario[10];
	  char texto[50];
	  char comando[10];
  };
  
  struct retorno
  {
	  int qtdRespostas;
	  char usuario[10];
	  char texto[50];
  };

	struct mensagens mensagens;
	struct retorno retorno;
  
	printf("Selecione a opcao desejada:\n");
	printf("1 - Cadastrar Mensagem\n");
	printf("2 - Ler Mensagens\n");
	printf("3 - Apagar Mensagens\n");
	printf("4 - Sair da Aplicacao\n\n");
	printf("Opcao: "); //Imprime as opcoes do Menu para o usuario
 
	scanf("%d", &opcao);               //Le a opcao escolhida pelo usuario

	printf("\n\n");
 
	switch(opcao)
	{
		case 1:         // opcao 1 = Enviar mensagens ao servidor
		{
			strcpy(mensagens.comando, "SalvaMsg");    // define o valor da variavel comando para ser reconhecida pelo servidor
     
			printf("Digite o Nome de Usuario: ");     // solicita que o usuario forneca as informacoes necessarias para a execucao do comando
			scanf("%s",mensagens.usuario);
			printf("Digite a Mensagem a ser Enviada: ");
			scanf("%s",mensagens.texto);
     
			printf("Enviando... Usuario: %s       Mensagem: %s\n",mensagens.usuario, mensagens.texto);
     
			break;
		}
   
		case 2:
		{
			strcpy(mensagens.comando, "ReturnMsg");    // define o valor da variavel comando para ser reconhecida pelo servidor
			strcpy(mensagens.texto, "");
			strcpy(mensagens.usuario, "");
		
			break;
		}
   
		case 3:
		{
			strcpy(mensagens.comando, "ApagaMsg");    // define o valor da variavel comando para ser reconhecida pelo servidor
			strcpy(mensagens.texto, "");
     
			printf("Digite o Nome de Usuario: ");     // solicita que o usuario forneca as informacoes necessarias para a execucao do comando
			scanf("%s",mensagens.usuario);
     
			break;
		}
   
		case 4:
		{
			strcpy(mensagens.comando, "CloseApp");
			strcpy(mensagens.texto, "");
			strcpy(mensagens.usuario, "");
     
			break;
		}
	}
  
		// Envia a struct contendo as iformacoes do comando para o servidor 
		if (send(s, &mensagens, sizeof(mensagens), 0) < 0)
		{
			perror("Send(struct)");
			exit(5);
		}

		if(strcmp(mensagens.comando, "CloseApp") == 0)
		{
			exit(0);
		}
    
		// Recebe a mensagem do servidor contendo a quantidade de respostas que serao recebidas e a primeira estrutura com respostas
		if(recv(s, &retorno, sizeof(retorno), 0) < 0)
		{
			perror("Recv(qtdRespostas)");
			exit(8);
		}
printf("**Teste.. qtdRespostas -> %d\n", retorno.qtdRespostas);
    
    
		//Se comando for igual a "SalvaMsg", apenas uma confirmacao de recebimento sera recebida
		if(strcmp(mensagens.comando, "SalvaMsg") == 0)
		{
			for(count = 0; count <= retorno.qtdRespostas; count++);
			{
				if(recv(s, &confirmacao, sizeof(confirmacao), 0) < 0)
				{
					perror("Recv(Confirmacao Envio)");
					exit(9);
				}
			}
       
			if(confirmacao == 0)
				printf("Mensagem nao salva no servidor, erro de recebimento!\n\n");
			else if (confirmacao == 1)
				printf("Mensagem salva no servidor com sucesso!\n\n");
			else if(confirmacao == 2)
				printf("Mensagem nao salva no servidor, memoria cheia!\n\n");
		}
    
		// Se comando for igual a "ReturnMsg", o  cliente recebera todas as mensagens salvas no servidor
		else if(strcmp(mensagens.comando, "ReturnMsg") == 0)
		{
			for(count = 1; count <= retorno.qtdRespostas; count++)
			{
				if(count != 1)   //count = 1, ja tem o primeiro retorno
				{
					if(recv(s, &retorno, sizeof(retorno), 0) < 0)
					{
						perror("Recv(Struct de Retorno op2)");
						exit(10);
					}
	
					printf("Mensagem %d\n", count);
					printf("Usuario: %s\n", retorno.usuario);
					printf("Mensagem: %s\n\n", retorno.texto);
				}
				else
				{
					printf("Mensagem %d\n", count);
					printf("Usuario: %s\n", retorno.usuario);
					printf("Mensagem: %s\n\n", retorno.texto);
				}
			}
		}
    
		else if(strcmp(mensagens.comando, "ApagaMsg") == 0)
		{
			printf("Mensagens apagadas: %d\n\n", retorno.qtdRespostas);
      
			for(count = 1; count <= retorno.qtdRespostas; count++)
			{
				if(count != 1)   //count = 1, ja tem o primeiro retorno
				{
					if(recv(s, &retorno, sizeof(retorno), 0) < 0)
					{
						perror("Recv(Struct de Retorn op3)");
						exit(12);
					}
	
					printf("Mensagem %d:\n", count);
					printf("Usuario: %s\n", retorno.usuario);
					printf("Mensagem: %s\n\n", retorno.texto);
				}
				else
				{
					printf("Mensagem %d:\n", count);
					printf("Usuario: %s\n", retorno.usuario);
					printf("Mensagem: %s\n\n", retorno.texto);
				}
			}
		}
    
    menu(s);
}
