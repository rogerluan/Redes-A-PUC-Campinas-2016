#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void recebeMsgs(int ns);

//Servidor TCP 
int main(int argc, char *argv[])
{
    unsigned short port;          
    struct sockaddr_in client, server; 
    int s;                     // Socket para aceitar conexões       
    int ns;                    // Socket conectado ao cliente        
    int namelen;          

    //Verifica se os parametros da aplicacao foram inseridos corretamente
    if (argc != 2)
    {
        printf("Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    //Cria um socket TCP (stream) para aguardar conexões     
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    // Define a qual endereço IP e porta o servidor estará ligado.
    // IP = INADDDR_ANY -> faz com que o servidor se ligue em todos os endereços IP    
    server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    //Liga o servidor à porta definida anteriormente.     
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
       perror("Bind()");
       exit(3);
    }

    
    //Prepara o socket para aguardar por conexões e cria uma fila de conexões pendentes.     
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }

    //Aceita uma conexão e cria um novo socket através do qual ocorrerá a comunicação com o cliente.     
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }

	recebeMsgs(ns);

    // Fecha o socket conectado ao cliente 
    close(ns);

    // Fecha o socket aguardando por conexões 
    close(s);
        
    return 0;
}

void recebeMsgs(int ns)
{
	//Variaveis usadas para o armazenamento das mensagens e para respostas ao cliente
    int qtdRespostas = 0, count = 0, qtdApagadas = 0, confirmacao = 0;
    
    struct mensagens
	{
		char usuario[10];
		char texto[50];
		char comando[10];
	};
  
	struct retorno
	{
		char usuario[10];
		char texto[50];
	};
	
	struct mensagens mensagens;
	struct mensagens mensagens1;
	struct mensagens mensagens2;
	struct mensagens mensagens3;
	struct mensagens mensagens4;
	struct mensagens mensagens5;
	struct retorno retorno;
	
    // Recebe uma mensagem do cliente através do novo socket conectado 
    if (recv(ns, &mensagens, sizeof(mensagens), 0) < 0)
    {
        perror("Recv(Struct Mensagens)");
        exit(6);
    }
    printf("\n**Comando recebido do cliente: %s\n", mensagens.comando);
    printf("Usuario: %s\n", mensagens.usuario);
    printf("Mensagem: %s\n\n", mensagens.texto);

    
    //Verifica qual foi o comando enviado e executa sua funcao
    //se comando = "SalvaMsg", a opcao 1 foi selecionada. Sendo assim as informacoes recebidas devem ser salvas
    if(strcmp(mensagens.comando, "SalvaMsg") == 0) 
    {		
		qtdRespostas = 0;
		
		for(count = 1; count <= 5; count++)
		{
			switch(count)
			{
				case 1:
				{
					if(strcmp(mensagens1.usuario, "") == 0)   //Mostra que o nome do primeiro usuario esta vazio, consequentemente tambem esta o texto
					{
						strcpy(mensagens1.usuario, mensagens.usuario);
						strcpy(mensagens1.texto, mensagens.texto);
						confirmacao = 1;
						count = 6;                  //Count = 6 faz com que o loop nao aconteca novamente para nao ahver duplicacao de mensagens
						qtdRespostas = 1;
					}					
					break;
				}
				case 2:
				{
					if(strcmp(mensagens2.usuario, "") == 0)
					{
						strcpy(mensagens2.usuario, mensagens.usuario);
						strcpy(mensagens2.texto, mensagens.texto);
						confirmacao = 1;
						count = 6;
						qtdRespostas = 1;
					}					
					break;
				}
				case 3:
				{
					if(strcmp(mensagens3.usuario, "") == 0)
					{
						strcpy(mensagens3.usuario, mensagens.usuario);
						strcpy(mensagens3.texto, mensagens.texto);
						confirmacao = 1;
						count = 6;
						qtdRespostas = 1;
					}					
					break;
				}
				case 4:
				{
					if(strcmp(mensagens4.usuario, "") == 0)
					{
						strcpy(mensagens4.usuario, mensagens.usuario);
						strcpy(mensagens4.texto, mensagens.texto);
						confirmacao = 1;
						count = 6;
						qtdRespostas = 1;
					}					
					break;
				}
				case 5:
				{
					if(strcmp(mensagens5.usuario, "") == 0)
					{
						strcpy(mensagens5.usuario, mensagens.usuario);
						strcpy(mensagens5.texto, mensagens.texto);
						confirmacao = 1;
						count = 6;
						qtdRespostas = 1;
					}					
					break;
				}
			}
			
			if(send(ns, &qtdRespostas, sizeof(qtdRespostas), 0) < 0)
			{
				perror("Send(1 -> confirm)");
				exit(9);
			} 
			
			if(qtdRespostas == 1)
			{
				if(send(ns, &confirmacao, sizeof(confirmacao), 0) < 0) //Envia a confirmacao de recebimento da mensagem para o cliente
				{
					perror("Send(confirmacao)");
					exit(9);
				}	
			}		
		}
	}
	// se comando = "ReturnMsg", a opcao 2 foi selecionada. Sendo assim todas as informacoes salvas devem ser retornadas
	else if(strcmp(mensagens.comando, "ReturnMsg") == 0) 
	{		
		qtdRespostas = 0;
		
		for(count = 1; count <= 5; count++)   //conta quantas posicoes sao diferentes de ""
		{
			switch(count)
			{
				case 1:
				{
					if(strcmp(mensagens1.usuario, "") != 0)
					{
						qtdRespostas++;
					}
					break;
				}
				case 2:
				{
					if(strcmp(mensagens2.usuario, "") != 0)
					{
						qtdRespostas++;
					}
					break;
				}
				case 3:
				{
					if(strcmp(mensagens3.usuario, "") != 0)
					{
						qtdRespostas++;
					}
					break;
				}
				case 4:
				{
					if(strcmp(mensagens4.usuario, "") != 0)
					{
						qtdRespostas++;
					}
					break;
				}
				case 5:
				{
					if(strcmp(mensagens5.usuario, "") != 0)
					{
						qtdRespostas++;
					}
					break;
				}
			}
		}

		if(send(ns, &qtdRespostas, sizeof(qtdRespostas), 0) < 0) // Envia qtdRespostas
		{
			perror("Send(qtdRespostas)");
			exit(10);
		}
		
		for(count = 1; count <= 5; count++)
		{
			switch(count)
			{
				case 1:
				{
					if(strcmp(mensagens1.usuario, "") != 0)   //Mostra que o nome do primeiro usuario esta vazio, consequentemente tambem esta o texto
					{
						strcpy(retorno.usuario, mensagens1.usuario);
						strcpy(retorno.texto, mensagens1.texto);
					}					
					break;
				}
				case 2:
				{
					if(strcmp(mensagens2.usuario, "") != 0)
					{
						strcpy(retorno.usuario, mensagens2.usuario);
						strcpy(retorno.texto, mensagens2.texto);
					}					
					break;
				}
				case 3:
				{
					if(strcmp(mensagens3.usuario, "") != 0)
					{
						strcpy(retorno.usuario, mensagens3.usuario);
						strcpy(retorno.texto, mensagens3.texto);
					}					
					break;
				}
				case 4:
				{
					if(strcmp(mensagens4.usuario, "") != 0)
					{
						strcpy(retorno.usuario, mensagens4.usuario);
						strcpy(retorno.texto, mensagens4.texto);
					}					
					break;
				}
				case 5:
				{
					if(strcmp(mensagens5.usuario, "") != 0)
					{
						strcpy(retorno.usuario, mensagens5.usuario);
						strcpy(retorno.texto, mensagens5.texto);
					}					
					break;
				}
			}
				
			if(send(ns, &retorno, sizeof(retorno), 0) < 0)
			{
				perror("Send(retorno RetornaTds)");
				exit(11);
			}			
		}
	}
	
	// se comando = "ApagaMsg", a opcao 3 foi selecionada. Sendo assim o servidor deve retornar todas as mensagens do usuario
    // fornecido para o cliente e em seguida apagar tais mensagens
	else if(strcmp(mensagens.comando, "ApagaMsg") == 0) 
	{
		qtdApagadas = 0;
		
		for(count = 1; count <= 5; count++) //Calcula quantas mensagens serao apagadas
		{
			switch(count)
			{
				case 1:
				{
					if(strcmp(mensagens1.usuario, mensagens.usuario) == 0)
					{
						qtdApagadas++;
					}
				}
				case 2:
				{
					if(strcmp(mensagens2.usuario, mensagens.usuario) == 0)
					{
						qtdApagadas++;
					}
				}
				case 3:
				{
					if(strcmp(mensagens3.usuario, mensagens.usuario) == 0)
					{
						qtdApagadas++;
					}
				}
				case 4:
				{
					if(strcmp(mensagens4.usuario, mensagens.usuario) == 0)
					{
						qtdApagadas++;
					}
				}
				case 5:
				{
					if(strcmp(mensagens5.usuario, mensagens.usuario) == 0)
					{
						qtdApagadas++;
					}
				}
			}
		}
		
		if(send(ns, &qtdApagadas, sizeof(qtdApagadas), 0) < 0)
		{
			perror("Send(qtdApagadas)");
			exit(13);
		}
		
		for(count = 1; count <= 5; count++)
		{
			switch(count)
				{
					case 1:
					{
						if(strcmp(mensagens1.usuario, mensagens.usuario) == 0)  //Mostra que em tal posicao, o nome do usuario eh igual ao nome recebido para apagar
						{
							strcpy(retorno.usuario, mensagens1.usuario);
							strcpy(retorno.texto, mensagens1.texto);
							
							strcpy(mensagens1.usuario, "");
							strcpy(mensagens1.texto, "");   //Apaga o valore das variaves
						}					
						break;
					}
					case 2:
					{
						if(strcmp(mensagens2.usuario, mensagens.usuario) == 0)
						{
							strcpy(retorno.usuario, mensagens2.usuario);
							strcpy(retorno.texto, mensagens2.texto);
							
							strcpy(mensagens2.usuario, "");
							strcpy(mensagens2.texto, "");
						}					
						break;
					}
					case 3:
					{
						if(strcmp(mensagens3.usuario, mensagens.usuario) == 0)
						{
							strcpy(retorno.usuario, mensagens3.usuario);
							strcpy(retorno.texto, mensagens3.texto);
							
							strcpy(mensagens3.usuario, "");
							strcpy(mensagens3.texto, "");
						}					
						break;
					}
					case 4:
					{
						if(strcmp(mensagens4.usuario, mensagens.usuario) == 0)
						{
							strcpy(retorno.usuario, mensagens4.usuario);
							strcpy(retorno.texto, mensagens4.texto);
							
							strcpy(mensagens4.usuario, "");
							strcpy(mensagens4.texto, "");
						}					
						break;
					}
					case 5:
					{
						if(strcmp(mensagens5.usuario, mensagens.usuario) == 0)
						{
							strcpy(retorno.usuario, mensagens5.usuario);
							strcpy(retorno.texto, mensagens5.texto);
							
							strcpy(mensagens5.usuario, "");
							strcpy(mensagens5.texto, "");
						}					
						break;
					}
				}
				
				if(send(ns, &retorno, sizeof(retorno), 0) < 0)
				{
					perror("Send(retorno Apaga)");
					exit(14);
				}
		}
	}    
    
	recebeMsgs(ns);
    
}


