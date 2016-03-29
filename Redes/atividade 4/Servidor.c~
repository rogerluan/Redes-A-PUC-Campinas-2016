#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define SHARED_MEMORY_ID 1322



	struct mensagem
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
	
	typedef struct
	{
		struct mensagem mensagem;
		struct mensagem mensagem1;
		struct mensagem mensagem2;
		struct mensagem mensagem3;
		struct mensagem mensagem4;
		struct mensagem mensagem5;		
	}memoria;

	memoria *Memoria;



void recebeMsgs(int s, struct sockaddr_in client);

//Servidor TCP 
int main(int argc, char *argv[])
{
    unsigned short port;          
    struct sockaddr_in client, server; 
    int s;                     // Socket para aceitar conexões

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
    // IP = INADDDR_ANY . faz com que o servidor se ligue em todos os endereços IP    
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

	recebeMsgs(s, client);

    // Fecha o socket aguardando por conexões 
    close(s);
        
    return 0;
}

void recebeMsgs(int s, struct sockaddr_in client)
{
	//Variaveis usadas para o armazenamento das Memoria->mensagem e para respostas ao cliente
    int count = 0, confirmacao = 0;    
    int ns;                    // Socket conectado ao cliente    
	socklen_t namelen;


	//Variaveis usadas para a Memoria Compartilhada
    int mem_id;
    key_t key = SHARED_MEMORY_ID;
    pid_t pid, fid;
    char *tmp_addr;
    
	
	struct retorno retorno;	
	
    //Cria Memoria
    if((mem_id = shmget(key, sizeof(Memoria), IPC_CREAT | 0666)) == -1)
    {
         printf("Impossivel criar a Memoria compartilhada!\n");
         exit(0);
    }
    
    //Associa ao segmento de Memoria compartilhada
    if( (tmp_addr = (char *)shmat(mem_id, NULL, 0)) == (char *)-1 )
    {
         printf("Impossivel associar ao segmento de Memoria compartilhada!\n");
         exit(1);
    }

    Memoria = (memoria *)tmp_addr; 


	//Inicializacao dos vetores
	retorno.qtdRespostas = 0;
	strcpy(retorno.texto, "");
	strcpy(retorno.usuario,"");
	strcpy(Memoria->mensagem.usuario, "");
	strcpy(Memoria->mensagem.texto, "");
	strcpy(Memoria->mensagem.comando, "");
	strcpy(Memoria->mensagem1.usuario, "");
	strcpy(Memoria->mensagem1.texto, "");
	strcpy(Memoria->mensagem1.comando, "");
	strcpy(Memoria->mensagem2.usuario, "");
	strcpy(Memoria->mensagem2.texto, "");
	strcpy(Memoria->mensagem2.comando, "");
	strcpy(Memoria->mensagem3.usuario, "");
	strcpy(Memoria->mensagem3.texto, "");
	strcpy(Memoria->mensagem3.comando, "");
	strcpy(Memoria->mensagem4.usuario, "");
	strcpy(Memoria->mensagem4.texto, "");
	strcpy(Memoria->mensagem4.comando, "");
	strcpy(Memoria->mensagem5.usuario, "");
	strcpy(Memoria->mensagem5.texto, "");
	strcpy(Memoria->mensagem5.comando, "");


   
   
  while(1)
  { 
    //Aceita uma conexão e cria um novo socket através do qual ocorrerá a comunicação com o cliente.     
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }
    
	if ((pid = fork()) == 0)
	{
	
		do
		{	//Processo filho
	      
			//Fecha o socket aguardando por conexões
			close(s);

			// Processo filho obtem seu próprio pid
			fid = getpid();


			// Recebe uma mensagem do cliente através do novo socket conectado 
			if (recv(ns, &Memoria->mensagem, sizeof(Memoria->mensagem), 0) < 0)
			{
				perror("Recv(Struct Memoria->mensagem)");
				exit(6);
			}
			
			printf("\n**Comando recebido do cliente: %s\n", Memoria->mensagem.comando);
			printf("Usuario: %s\n", Memoria->mensagem.usuario);
			printf("Mensagem: %s\n\n", Memoria->mensagem.texto);

		
			//Verifica qual foi o comando enviado e executa sua funcao
			//se comando = "SalvaMsg", a opcao 1 foi selecionada. Sendo assim as informacoes recebidas devem ser salvas
			if(strcmp(Memoria->mensagem.comando, "SalvaMsg") == 0) 
			{		
				retorno.qtdRespostas = 0;
				
				for(count = 1; count <= 5; count++)
				{
					switch(count)
					{
						case 1:
						{
							if(strcmp(Memoria->mensagem1.usuario, "") == 0)   //Mostra que o nome do primeiro usuario esta vazio, consequentemente tambem esta o texto
							{
								strcpy(Memoria->mensagem1.usuario, Memoria->mensagem.usuario);
								strcpy(Memoria->mensagem1.texto, Memoria->mensagem.texto);
								confirmacao = 1;
								count = 6;                  //Count = 6 faz com que o loop nao aconteca novamente para nao ahver duplicacao de Memoria->mensagem
								retorno.qtdRespostas = 1;
							}					
							break;
						}
						case 2:
						{
							if(strcmp(Memoria->mensagem2.usuario, "") == 0)
							{
								strcpy(Memoria->mensagem2.usuario, Memoria->mensagem.usuario);
								strcpy(Memoria->mensagem2.texto, Memoria->mensagem.texto);
								confirmacao = 1;
								count = 6;
								retorno.qtdRespostas = 1;
							}					
							break;
						}
						case 3:
						{
							if(strcmp(Memoria->mensagem3.usuario, "") == 0)
							{
								strcpy(Memoria->mensagem3.usuario, Memoria->mensagem.usuario);
								strcpy(Memoria->mensagem3.texto, Memoria->mensagem.texto);
								confirmacao = 1;
								count = 6;
								retorno.qtdRespostas = 1;
							}					
							break;
						}
						case 4:
						{
							if(strcmp(Memoria->mensagem4.usuario, "") == 0)
							{
								strcpy(Memoria->mensagem4.usuario, Memoria->mensagem.usuario);
								strcpy(Memoria->mensagem4.texto, Memoria->mensagem.texto);
								confirmacao = 1;
								count = 6;
								retorno.qtdRespostas = 1;
							}					
							break;
						}
						case 5:
						{
							if(strcmp(Memoria->mensagem5.usuario, "") == 0)
							{
								strcpy(Memoria->mensagem5.usuario, Memoria->mensagem.usuario);
								strcpy(Memoria->mensagem5.texto, Memoria->mensagem.texto);
								confirmacao = 1;
								count = 6;
								retorno.qtdRespostas = 1;
							}					
							break;
						}
					}
				
					if(send(ns, &retorno.qtdRespostas, sizeof(retorno.qtdRespostas), 0) < 0)
					{
						perror("Send(1 . confirm)");
						exit(9);
					} 
				
					if(retorno.qtdRespostas == 1)
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
			else if(strcmp(Memoria->mensagem.comando, "ReturnMsg") == 0) 
			{		
				retorno.qtdRespostas = 0;
			
				for(count = 1; count <= 5; count++)   //conta quantas posicoes sao diferentes de ""
				{
					switch(count)
					{
						case 1:
						{
							if(strcmp(Memoria->mensagem1.usuario, "") != 0)
							{
								retorno.qtdRespostas++;
							}
							break;
						}
						case 2:
						{
							if(strcmp(Memoria->mensagem2.usuario, "") != 0)
							{
								retorno.qtdRespostas++;
							}
							break;
						}
						case 3:
						{
							if(strcmp(Memoria->mensagem3.usuario, "") != 0)
							{
								retorno.qtdRespostas++;
							}
							break;
						}
						case 4:
						{
							if(strcmp(Memoria->mensagem4.usuario, "") != 0)
							{
								retorno.qtdRespostas++;
							}
							break;
						}
						case 5:
						{
							if(strcmp(Memoria->mensagem5.usuario, "") != 0)
							{
								retorno.qtdRespostas++;
							}
							break;
						}
					}
				}
			
				for(count = 1; count <= 5; count++)
				{
					switch(count)
					{
						case 1:
						{
							if(strcmp(Memoria->mensagem1.usuario, "") != 0)   //Mostra que o nome do primeiro usuario nao esta vazio, consequentemente tambem nao esta o texto
							{
								strcpy(retorno.usuario, Memoria->mensagem1.usuario);
								strcpy(retorno.texto, Memoria->mensagem1.texto);
							}					
							break;
						}
						case 2:
						{
							if(strcmp(Memoria->mensagem2.usuario, "") != 0)
							{
								strcpy(retorno.usuario, Memoria->mensagem2.usuario);
								strcpy(retorno.texto, Memoria->mensagem2.texto);
							}					
							break;
						}
						case 3:
						{
							if(strcmp(Memoria->mensagem3.usuario, "") != 0)
							{
								strcpy(retorno.usuario, Memoria->mensagem3.usuario);
								strcpy(retorno.texto, Memoria->mensagem3.texto);
							}					
							break;
						}
						case 4:
						{
							if(strcmp(Memoria->mensagem4.usuario, "") != 0)
							{
								strcpy(retorno.usuario, Memoria->mensagem4.usuario);
								strcpy(retorno.texto, Memoria->mensagem4.texto);
							}					
							break;
						}
						case 5:
						{
							if(strcmp(Memoria->mensagem5.usuario, "") != 0)
							{
								strcpy(retorno.usuario, Memoria->mensagem5.usuario);
								strcpy(retorno.texto, Memoria->mensagem5.texto);
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
		
			// se comando = "ApagaMsg", a opcao 3 foi selecionada. Sendo assim o servidor deve retornar todas as Memoria->mensagem do usuario
			// fornecido para o cliente e em seguida apagar tais Memoria->mensagem
			else if(strcmp(Memoria->mensagem.comando, "ApagaMsg") == 0) 
			{
				retorno.qtdRespostas = 0;
			
				for(count = 1; count <= 5; count++) //Calcula quantas Memoria->mensagem serao apagadas
				{
					switch(count)
					{
						case 1:
						{
							if(strcmp(Memoria->mensagem1.usuario, Memoria->mensagem.usuario) == 0)
							{
								retorno.qtdRespostas++;
							}
						}
						case 2:
						{
							if(strcmp(Memoria->mensagem2.usuario, Memoria->mensagem.usuario) == 0)
							{
								retorno.qtdRespostas++;
							}
						}
						case 3:
						{
							if(strcmp(Memoria->mensagem3.usuario, Memoria->mensagem.usuario) == 0)
							{
								retorno.qtdRespostas++;
							}
						}
						case 4:
						{
							if(strcmp(Memoria->mensagem4.usuario, Memoria->mensagem.usuario) == 0)
							{
								retorno.qtdRespostas++;
							}
						}
						case 5:
						{
							if(strcmp(Memoria->mensagem5.usuario, Memoria->mensagem.usuario) == 0)
							{
								retorno.qtdRespostas++;
							}
						}
					}
				}
				
				for(count = 1; count <= 5; count++)
				{
					switch(count)
						{
							case 1:
							{
								if(strcmp(Memoria->mensagem1.usuario, Memoria->mensagem.usuario) == 0)  //Mostra que em tal posicao, o nome do usuario eh igual ao nome recebido para apagar
								{
									strcpy(retorno.usuario, Memoria->mensagem1.usuario);
									strcpy(retorno.texto, Memoria->mensagem1.texto);
									
									strcpy(Memoria->mensagem1.usuario, "");
									strcpy(Memoria->mensagem1.texto, "");   //Apaga o valor das variaves
								}					
								break;
							}
							case 2:
							{
								if(strcmp(Memoria->mensagem2.usuario, Memoria->mensagem.usuario) == 0)
								{
									strcpy(retorno.usuario, Memoria->mensagem2.usuario);
									strcpy(retorno.texto, Memoria->mensagem2.texto);
									
									strcpy(Memoria->mensagem2.usuario, "");
									strcpy(Memoria->mensagem2.texto, "");
								}					
								break;
							}
							case 3:
							{
								if(strcmp(Memoria->mensagem3.usuario, Memoria->mensagem.usuario) == 0)
								{
									strcpy(retorno.usuario, Memoria->mensagem3.usuario);
									strcpy(retorno.texto, Memoria->mensagem3.texto);
									
									strcpy(Memoria->mensagem3.usuario, "");
									strcpy(Memoria->mensagem3.texto, "");
								}					
								break;
							}
							case 4:
							{
								if(strcmp(Memoria->mensagem4.usuario, Memoria->mensagem.usuario) == 0)
								{
									strcpy(retorno.usuario, Memoria->mensagem4.usuario);
									strcpy(retorno.texto, Memoria->mensagem4.texto);
									
									strcpy(Memoria->mensagem4.usuario, "");
									strcpy(Memoria->mensagem4.texto, "");
								}					
								break;
							}
							case 5:
							{
								if(strcmp(Memoria->mensagem5.usuario, Memoria->mensagem.usuario) == 0)
								{
									strcpy(retorno.usuario, Memoria->mensagem5.usuario);
									strcpy(retorno.texto, Memoria->mensagem5.texto);
									
									strcpy(Memoria->mensagem5.usuario, "");
									strcpy(Memoria->mensagem5.texto, "");
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
	    } while(strcmp(Memoria->mensagem.comando,"CloseApp") != 0);
    
		/* Fecha o socket conectado ao cliente */
		close(ns);

		/* Processo filho termina sua execução */
		printf("[%d] Processo filho terminado com sucesso.\n", fid);
		exit(0);
	}
	else
	{  
		//Processo pai
		if (pid > 0)
		{
			printf("Processo filho criado: %d\n", pid);
				
				//Fecha o socket conectado ao cliente
			close(ns);
		}
		else
		{
			perror("Fork()");
			exit(8);	      
		}
	}    
  }
}



