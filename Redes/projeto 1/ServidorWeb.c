#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include <netinet/in.h>

#define port "50000"				
#define webroot "/home/pedro/Desktop/ArquivosRedes"	

//Prototipos
int tamanhoArquivo(int ns);
void enviaMsg(int ns, char *msg);
int recebeSolicitacao(int ns, char *buffer);
						
int main()
{
	//Declaracao de Variaveis
	int s,ns, err;
	void *addr;
	struct addrinfo *respostas, *p, tiposSuportados;
	struct sockaddr_storage cliente;
	socklen_t tam_end;
	char solicitacao[500], recurso[500], *ponteiro, ip[INET6_ADDRSTRLEN];		
	int ns1, tamanho, ok = 1;
	pid_t pid, fid;

	//popula a variavel tiposSuportados com Zeros
	memset(&tiposSuportados, 0, sizeof(tiposSuportados));
	
	//Definicao dos tipos suportados pelo socket
	tiposSuportados.ai_family=AF_UNSPEC;
	tiposSuportados.ai_flags=AI_PASSIVE;
	tiposSuportados.ai_socktype=SOCK_STREAM;
	
	printf("Servidor aberto na porta 50000\n");
	
	//Cria e define as opcoes do Socket
	if((err = getaddrinfo(NULL, port, &tiposSuportados, &respostas)) == -1)
	{
		printf("Erro -> getaddrinfo : %s\n", gai_strerror(err));
		return(1);
	}

	for(p = respostas; p != NULL; p = p->ai_next)
	{		
		if((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			printf("Erro -> Criacao do Socket\n");
			continue;
		}

		if(bind(s, p->ai_addr, p->ai_addrlen) == -1)
		{
			printf("Erro -> Bind(s)\n");
			close(s);			
			continue;
		}
		
		break;
	}

	if(listen(s, 15) == -1)
	{
		printf("Erro -> Listen\n");
		return(1);
	}
	
	//Realiza um accept no socket ns e aceita a conexao
	while(1)
	{
	
		tam_end = sizeof(cliente);
		if((ns = accept(s, (struct sockaddr *)&cliente,&tam_end)) == -1)
		{
			printf("Erro -> Accept(ns)\n");
			return(1);
		}
		
		if ((pid = fork()) == 0)
		{
		//Processo filho 
		 
	      
		// Fecha o socket aguardando por conexões 
		close(s);

		// Processo filho obtem seu próprio pid 
		fid = getpid();
	
			for(p = respostas; p != NULL; p = p->ai_next)
			{			
				if(p->ai_family == AF_INET)
				{
					struct sockaddr_in *ip;
					ip = (struct sockaddr_in *)p->ai_addr;
					addr = &(ip->sin_addr);
				}
	
				if(p->ai_family == AF_INET6)
				{
					struct sockaddr_in6 *ip;
					ip = (struct sockaddr_in6 *)p->ai_addr;
					addr = &(ip->sin6_addr);
				}
				
				inet_ntop(p->ai_family, addr, ip, sizeof(ip));
				printf("Conexao estabelecida com: %s\n",ip);
			}
	
			//Chama a funcao responsavel por receber solicitacoes
			if(recebeSolicitacao(ns, solicitacao) == 0)
			{
				printf("Erro -> Rcv\n");
			}
		
			// Checa se a solicitacao do navegador eh valida
			ponteiro = strstr(solicitacao, " HTTP/");    //Verifica se "HTTP/" faz parte da string solicitacao
	
			if(ponteiro == NULL)
			{
				printf("Solicitacao invalida (Nao eh HTTP!)\n");
			}
			else  //Sendo uma solicitacao valida, checa o tipo de solicitacao
			{
				*ponteiro= 0;
				ponteiro= NULL;
		
			
				if(strncmp(solicitacao, "GET ", 4) == 0)
					ponteiro=solicitacao+4;
	
				if(strncmp(solicitacao, "HEAD ", 5) == 0)
					ponteiro=solicitacao+5;
	
				if(ponteiro == NULL)
					printf("Solicitacao Desconhecida \n");
	
				else
				{
					if(ponteiro[strlen(ponteiro) - 1] == '/')
						strcat(ponteiro, "index.html");   //Copia "index.html" para o inicio de ponteiro
	
					strcpy(recurso, webroot);
					strcat(recurso, ponteiro);
	
					ns1 = open(recurso, O_RDONLY, 0);  //abre ns1 apenas para leitura (O_RDONLY)
	
					printf("Abrindo \"%s\"\n", recurso);
					if(ns1 == -1)
					{
						printf("404 File not found Error\n");
						enviaMsg(ns,"HTTP/1.0 404 Not Found\r\n");
						enviaMsg(ns,"Server : Projeto1 - Redes\r\n\r\n");
						enviaMsg(ns,"<html><head><title>404 not found error!! :(</head></title>");
						enviaMsg(ns,"<body><h1>Url not found</h1><br><p>Sorry user the url you were searching for was not found on this server!!</p><br><br><br><h1>Projeto1 - Redes</h1></body></html>");
					}
					else
					{
						printf("200 OK!!!\n");
						enviaMsg(ns,"HTTP/1.0 200 OK\r\n");
						enviaMsg(ns,"Server : Projeto1 - Redes\r\n\r\n");
						if(ponteiro == solicitacao+4)			 // Se a solicitacao eh um GET
						{
							if((tamanho = tamanhoArquivo(ns1)) == -1)
								printf("Erro ao verificar tamanho \n");
							if((ponteiro = (char *)malloc(tamanho)) == NULL)
								printf("Erro ao alocar memoria!!\n");
							read(ns1, ponteiro, tamanho);
				
							if(send(ns, ponteiro, tamanho, 0) == -1)
							{
								printf("Erro -> Send!!\n");
							}
							free(ponteiro);
						}
					}
					close(ns);
				}
			}
			shutdown(ns,SHUT_RDWR);
			
			// Processo filho termina sua execução 
			printf("[%d] Processo filho terminado com sucesso.\n", fid);
			exit(0);
		}
		else
			//Processo pai 
		 
		if (pid > 0)
		{
		    printf("Processo filho criado: %d\n", pid);

		    // Fecha o socket conectado ao cliente 
		    close(ns);
		}
		else
		{
		    perror("Fork()");
		    exit(8);	      
		}
	}

	freeaddrinfo(respostas);
	close(ns);
	close(s);
	return(0);
}

//Checa o tamanho do arquivo a ser enviado
int tamanhoArquivo(int ns)
{
	struct stat stat_struct;
	if(fstat(ns, &stat_struct) == -1)
		return(1);
	return (int)stat_struct.st_size;
}

//Envia a resposta da solicitacao
void enviaMsg(int ns, char *msg)
{
	int tam = strlen(msg);
	if(send(ns,msg,tam,0) == -1)
	{
		printf("Error in send\n");
	}
}

//Recebe a Solicitacao
int recebeSolicitacao(int ns, char *solicitacao)
{
	#define EndOfLine "\r\n"
	#define EndOfLine_SIZE 2
	char *p = solicitacao;
	int aux = 0;			// usado para verificar se o buffer e a nova mensagem sao equivatamtes
	
	while(recv(ns,p,1,0)!=0)		// recebe byte por byte
	{
		if(*p == EndOfLine[aux])	// Se o primeiro byte for /r, verifica se eh o fim da mensagem (/r)
		{	
			++aux;		
			if(aux == EndOfLine_SIZE)
			{
				*(p+1-EndOfLine_SIZE) = '\0';	// finaliza a string
				return(strlen(solicitacao));	// retorna os bytes recebidos
			}
		}
		else
		{
			aux = 0;			
		}
		p++;					// incrementa p para receber o proximo byte
	}
	return(0);
}
