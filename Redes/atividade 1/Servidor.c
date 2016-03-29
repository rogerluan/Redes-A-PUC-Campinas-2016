#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void funcServidor(struct sockaddr_in server, int s)
{
	struct sockaddr_in client;
	int qtdMsgs, client_address_size;
	char texto[50], usuario[10];
  int count, flagMensagens, qtdMsgs_size;
  char mensagens[5][2], posVazia;
		
  //Inicializacao das variaveis
  flagMensagens = 0;
  count = 0;
  qtdMsgs = 0;
  strcpy(&posVazia, "");
	
  for(count = 0; count < 5; count++)    //Inicializa a Matriz com todas as posicoes contendo a string posVazia
  {
    mensagens[count][1] = posVazia;
    mensagens[count][2] = posVazia;
  }

  //Recebe uma mensagem do cliente. O endereço do cliente será armazenado em "client".
  client_address_size = sizeof(client);;
  recvfrom(s, usuario, sizeof(usuario), 0, (struct sockaddr *) &client, &client_address_size);
   
  if(strcmp(usuario,"ReturnMsg") == 0)   //Se a opcao 2 no cliente for selecionada, a primeira informacao enviada sera "ReturnMsg"
                                          //Ao recebe-la o servidor retorna o que tem salvo em memoria
	{
	   printf("\nOpcao 2 selecionada. Mensagens Armazenadas:\n");
		
	   for(count = 0; count < 5; count++)
	   {
		    printf("Mensagem %d\n", count+1);
		    printf("Nome: %s\n", &mensagens[count][1]);
		    printf("Mensagem: %s\n", &mensagens[count][2]);
	   }

	   for(count = 0; count < 5; count++)
	   {
        if(strcmp(&mensagens[count][1], &posVazia) != 0)     //Se a informacao contida for diferente de posVazia, existe algo salvo em tal posicao
	      {
		      qtdMsgs++;         //Conta quantas mensagens estao salvas no servidor para nao enviar mais dados do que for necessario
		      qtdMsgs_size = sizeof(qtdMsgs);
		    }
	   }
        
     //Envia o numero de mensagens salvas para que o cliente receba a quantidade correta
     if(sendto(s, &qtdMsgs, qtdMsgs_size, 0, (struct sockaddr *)&client, client_address_size) < 0)
     {
		    perror("sendto(qtdMsgs)");
		    exit(1);
     }            
            
     for(count = 0; count <= qtdMsgs; count++) //Envia ao Cliente todas as mensagens salvas e quem as enviou
     {
	      sendto(s, &mensagens[count][1], (strlen(&mensagens[count][1])+1), 0, (struct sockaddr *)&client, client_address_size);
        sendto(s, &mensagens[count][2], (strlen(&mensagens[count][2])+1), 0, (struct sockaddr *)&client, client_address_size);
	   }	
	}
  else //Se nao for recebida a mensagem "ReturnMsg", uma mensagem real esta sendo enviada
  {				
    recvfrom(s, texto, sizeof(texto), 0, (struct sockaddr *) &client, &client_address_size);
    printf("\n***Mensagem recebida com sucesso***\nNome: %s\n", usuario);  //Imprime a mensagem para que seja possivel uma confirmacao pelo servidor
	  printf("Mensagem: %s\n", texto);
		
	  //Verifica a primeira posicao vazia na matriz para armazenar as mensagens
	  for(count = 0; count < 5; count++)
	  {
	      if(strcmp(&mensagens[count][1], &posVazia) == 0)  //Armazena os dados da mensagem na primeira linha vazia da matriz
		  	{
			   	strcpy(&mensagens[count][1], usuario);
				  strcpy(&mensagens[count][2], texto);              //SUPONHO QUE SEJA NESTA PASSAGEM QUE ESTA DANDO ERRO, QUANDO TENTO IMPRIMIR AS
					                                       					  //POSICOES DA MATRIZ, ELAS CONSTAM VAZIAS. 
        }
		}
	}
	funcServidor(server, s);
				
  close(s);
}

//Servidor UDP
int main(int argc, char *argv[])
{
  struct sockaddr_in server;
  int porta, s;   
   
  //Verifica se os parametros para a funcao foram usados corretamente
  if(argc != 2)
  {
    printf("Use: %s NumeroPorta\n",argv[0]);
    exit(1);
  }
  porta = htons(atoi(argv[1]));


  //Define a qual endereço IP e porta o servidor estará ligado.
  server.sin_family      = PF_INET;       
  server.sin_port        = porta;         
  server.sin_addr.s_addr = INADDR_ANY;    

  //Cria um socket UDP (dgram). 
  if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket()");
    exit(1);
  }
		
  //Liga o servidor à porta definida anteriormente.
  if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("bind()");
    exit(1);
  }

  funcServidor(server, s);

  return 0;
}
