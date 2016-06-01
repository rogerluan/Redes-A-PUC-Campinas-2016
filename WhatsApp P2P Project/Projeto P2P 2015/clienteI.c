/* Cliente.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <crypt.h>


struct mensagem
{ 
  int control;      
  int flag;        /* 0, 1, 2 */
  char login[10];  /* usuario */
  char senha[14]; /* senha */
  char palavra_Chave[20];
  char data[50];
}enviar;

struct arquivos
{
  char hostname[20];
  char porta[5];
  char nome[5][20];
  char desc[5][50];
  int  numero;
  struct arquivos *prox;
}arqs;



void *servidor_cliente()
{
  int s;
  int ns;
  int namelen;
  int tam_transf; /* tamanho do arquivo a ser transferido */
  unsigned short port;
  struct sockaddr_in server; 
  struct sockaddr_in client;
  struct arquivos busca[20];
  struct stat filestatus;
  FILE *transf;
  char transferido[100000];
  char path[100];
  

  srand(time(NULL));
  port = rand() % 20 + 5001; /* Numero entre 5001 e 5020 */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
      perror("Socket()");
      exit(2);
  }

  /*
   * Define a qual endereço IP e porta o servidor estará ligado.
   * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
   * os endereços IP
   */
  server.sin_family = AF_INET;   
  server.sin_port   = htons(port);       
  server.sin_addr.s_addr = INADDR_ANY;

  /*
   * Liga o servidor à porta definida anteriormente.
   */
  if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
   perror("Bind()");
   exit(3);
  }

  /*
   * Prepara o socket para aguardar por conexões e
   * cria uma fila de conexões pendentes.
   */
  if (listen(s, 1) != 0)
  {
    perror("Listen()");
    exit(4);
  }
  
  strcpy(arqs.hostname, "localhost");
  sprintf(arqs.porta, "%d", port);

  /*
   * Aceita uma conexão e cria um novo socket através do qual
   * ocorrerá a comunicação com o cliente.
   */
  namelen = sizeof(client);

  do{

		    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
		    {
		      perror("Accept()");
		      exit(5);
		    }

		    if(recv(ns, &busca, sizeof(struct arquivos), 0) == -1)
		    {
		      perror("Recv()");
		      exit(6);
		    }
		    
		    strcpy(path, "./Compartilhado");
		    strcat(path, busca[0].nome[0]); /* coloca em path nome do arquivo que vai ser transferido */

		    if((transf = fopen(path, "r")) == NULL)
		      puts("Erro abrir arquivo");

		    stat(path, &filestatus); /* Obtem tamanho do arquivo que foi aberto anteriormente */
		    fread(transferido, 1, filestatus.st_size, transf); /* Le o arquivo e coloca seu conteudo em transferido */
		    fclose(transf);
		    tam_transf = filestatus.st_size;

		    if(send(ns, &tam_transf, sizeof(tam_transf), 0) < 0)
		    {
		      perror("Send()");
		      exit(7);
		    }

		    if(send(ns, &transferido, sizeof(transferido), 0) < 0)
		    {
		      perror("Send()");
		      exit(7);
		    }

  }while(1);
}

int conectar(char *hostname, char *porta)
{

	  unsigned short port;           
	  struct hostent *hostnm; 
	  struct sockaddr_in server;
	  int s;

	  /*
	   * Obtendo o endereço IP do servidor
	   */
	  hostnm = gethostbyname(hostname);
	  if (hostnm == (struct hostent *) 0)
	  {
	    fprintf(stderr, "Gethostbyname failed\n");
	    exit(2);
	  }
	  port = (unsigned short) atoi(porta);
	 
	  /*
	   * Define o endereço IP e a porta do servidor
	   */
	  server.sin_family      = AF_INET;
	  server.sin_port        = htons(port);
	  server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

	  /*
	   * Cria um socket TCP (stream)
	   */
	  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	  {
	    perror("Socket()");
	    exit(3);
	  }

	  /* Estabelece conexão com o servidor */
	  if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	  {
	    perror("Connect()");
	    exit(4);
	  }  /* Envia a mensagem no buffer de envio para o servidor */
	  return s;

}

int login(int s, char *hostname)
{

  int i=0;
  char *pass2; //pass encriptado
  char *aux = "af";
  char nada[2]; //leitura da linha em branco de compartilhados.txt
  FILE *fl;
  
  
  /* Popula struct mensagem com dados para Login */
  printf("Login: ");
  scanf("%s", enviar.login);
  
  pass2 = crypt(getpass("Senha: "), aux);
  
  strcpy(enviar.senha, pass2);

  enviar.control = 0;
  enviar.flag = 0;

  /* Envia dados para login */
  if(send(s, &enviar, sizeof(struct mensagem), 0) < 0)
  {
    perror("Send()");
    exit(7);
  }

  // Recebe resposta e verifica se foi 0 
  if(recv(s, &enviar, sizeof(struct mensagem), 0) == -1)
  {
    perror("Recv()");
    exit(6);
  }

  if(enviar.flag == 2 || enviar.flag == 0)
  {    
		    fl = fopen("./Compartilhado/compartilhados.txt", "r");
		    if(fl == NULL)
		    {
		      perror("Erro!");
		      exit(6);
		    }
		    
		    /* Preenche estrutura arqs com os dados do arquivo "compartilhados.txt" */
		    strcpy(arqs.hostname, hostname);
		    
		    while(!feof(fl))
		    {
		      
			      fgets(arqs.nome[i], sizeof(arqs.nome[i]), fl);
			      fgets(arqs.desc[i], sizeof(arqs.desc[i]), fl);
			      fgets(nada, sizeof(nada), fl);
			      i++;
			      printf("\nNome: %sDescricao: %s\n", arqs.nome[i], arqs.desc[i]);
		    }
		    	  arqs.numero = i;
		    
		    /* Envia hostname, arquivos e suas descrições (estrutura arqs) */
		    fprintf(stdout,"Arqs.nome==%s",arqs.nome[0]);
		    if(send(s, &arqs, sizeof(struct arquivos), 0) < 0)
		    {
		      perror("Send()");
		      exit(7);
		    } 
		    fclose(fl);


  }  
 return(enviar.flag);
}

void logout(int s)
{
	  enviar.control = 4;

	  if(send(s, &enviar, sizeof(struct mensagem), 0) < 0)
	  {
	    perror("Send()");
	    exit(7);
	  }

	  puts("Fazendo logout...");
	  sleep(1);
	  system("clear");
}

int receber(int s,char *nome_aux)
{
    enviar.control = 40;

    int bytesReceived = 0;
    char recvBuff[256]; 
    char name[20];

    strcpy(enviar.data, nome_aux);
    if (send(s, &enviar, sizeof(struct mensagem), 0) < 0)
    {
		perror("Send()");
		exit(5);
    }

    //recebe o nome do arquivo será requisitado ao servidor

    memset(recvBuff, '0', sizeof(recvBuff));

    if (recv(s, &enviar, sizeof(struct mensagem), 0) == -1)
    {
          perror("Recv()");
          exit(6);
    }

    FILE *fp;
    fp = fopen(enviar.data, "w"); 

    if(NULL == fp)
    {
        printf("Error opening file");
        return -1;
    }
    
    /* Recebe o numero de bytes do arquivo */
    bytesReceived = read(s, recvBuff, 256);
    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
	
    /*Escreve o conteudo do arquivo requisitado no arquivo local e Finaliza*/
    fwrite(enviar.data, 1,strlen(enviar.data)+1,fp);

    printf("Arquivo Recebido\n");   
}

/* Procedimento usado para buscar por palavras chaves */
void buscar(int s)
{
	  int tam_transf;
	  int i = 1;
	  int j;
	  int k = 0;
	  int ss;
	  char nome_aux[40];
	  struct stat filestatus;
	  struct arquivos busca[20]; /* Todos arquivos encontrados pela busca */
	  struct arquivos escolhido; /* Arquivo que será transferido */
	  char transferido[100000];
	  char path[100] = "./Compartilhado";
	  FILE *transf;
	  
	  char resp = 'n'; /* indica se quer ou nao transferir arquivo encontrado */
	  char palavra_Chave[20];

	  enviar.control = 2;
	  system("clear");
	  puts("Introduza a palavra chave para buscar: ");
	  __fpurge(stdin);
	  fgets(enviar.palavra_Chave, sizeof(enviar.palavra_Chave), stdin);
	  __fpurge(stdin);
	  enviar.palavra_Chave[strlen(enviar.palavra_Chave)-1] = '\0';

	  /* Envia palavras chave para  o servidor central realizar busca */
	  if(send(s, &enviar, sizeof(struct mensagem), 0) < 0)
	  {
	    perror("Send()");
	    exit(7);
	  }
	  

	  if(recv(s, &busca, (sizeof(struct arquivos) * 20), 0) == -1)
	  {
	    perror("Recv()");
	    exit(6);
	  }


	  if(strcmp(busca[0].hostname, "NADA") == 0)
	  {
	    puts("Nenhum arquivo encontrado.\n");
	    sleep(1);
	    return;
	  }

	  while(strcmp(busca[k].hostname, "NADA") != 0)
	    k++;

	  printf("%d arquivos encontrados.\n\n", k);
	  /* Lista todos arquivos encontrados na busca */  
	  for(k = 0; strcmp(busca[k].hostname, "NADA") != 0; k++)
	  {
		    for(j = 0; j < busca[k].numero != 0; j++)
		    {
			      puts("Deseja transferir este arquivo? (s/n)");
			      printf("%d- Nome: %s   Descricao: %s\n", i, busca[k].nome[j], busca[k].desc[j]);
			      printf("Usuario que esta compartilhando\n");
			      printf("IP: %s\n",busca[k].hostname);
			      printf("Opcao:");
			      __fpurge(stdin);
			      scanf("%c", &resp);
                              __fpurge(stdin);
			      if(resp == 's')
			      {
				        strcpy(busca[0].hostname, busca[k].hostname);
				        strcpy(busca[0].porta, busca[k].porta);
				        strcpy(busca[0].nome[0], busca[k].nome[j]);
				        break;
			      }
		      	else if(resp == 'n')
		        {  
						break;
		        }
		      i++;
		    }
	       if(resp == 's')
	       {
		       strcpy(nome_aux, busca[0].nome[0]);
		       receber(s, nome_aux);
               }
		
	  }

	  if(resp != 'n')
	  {  
			  if(j == busca[k].numero)
			  {
			      puts("Nenhum arquivo transferido.");
			      return;
			  }

			  ss = conectar(busca[0].hostname, busca[0].porta);

			  /* Deixa nome do arquivo certo (tira '\n' do fim) */
			  busca[0].nome[0][strlen(busca[0].nome[0])-1] = '\0';
			  
			  if(send(ss, &busca, sizeof(struct arquivos), 0) < 0)
			  {
			    perror("Send()");
			    exit(7);
			  }
			  

			  if(recv(ss, &tam_transf, sizeof(tam_transf), 0) == -1)
			  {
			    perror("Recv()");
			    exit(6);
			  }

			  if(recv(ss, &transferido, sizeof(transferido), 0) == -1)
			  {
			    perror("Recv()");
			    exit(6);
			  }
			  
			  strcat(path, busca[0].nome[0]);

			  transf = fopen(path, "w");
			  fwrite(transferido, 1, tam_transf, transf);
			  fclose(transf);

			  puts("Arquivo transferido com sucesso!");
	  }else
	  {
	    system("clear");
	  }
}

int main(int argc, char **argv)
{
	  int id;
	  int s;
	  int opcao; /* Escolha do cliente */
	  pthread_t threads[5];
	    
	  s = conectar(argv[1], argv[2]);
	  /* Cria thread com servidor_cliente 
	   * Esta thread que será usada para transferir arquivos para
	   * outros clientes 
	   */
	  pthread_create(&threads[0], NULL, servidor_cliente, NULL);

	  do{
		    id = login(s, argv[1]);
		    if(id == 1)
		      printf("Usuario e senha incorreto!\n");
		      sleep(1);

	    }while(id == 1);
	  
	  
	  if(id == 0)
	    printf("Login feito com sucesso!\n");
	    sleep(1);
	  if(id == 2)
	    printf("Novo usuario cadastrado!\n");
	    sleep(1);

	  do{
	    		system("clear");
	   			printf("Opcoes: \n");
	  			printf("[1] - Buscar \n");
	  			printf("[2] - Receber \n");
	  			printf("[3] - Enviar \n");
				printf("[4] - Logout \n");
	  			printf("[5] - Sair \n");
				printf("[6] - Listar usuarios \n");
	      		printf("[7] - Listar arquivos\n");
	  			scanf("%d", &opcao);

			    switch(opcao)
			    {
					      case 1:
						  buscar(s);
					      break;

					      case 2:
						//Receber
					      break;
					      
					      case 3:
						//Enviar
					      break;
					      
					      case 4:
					      logout(s);
					      return 0;
					      break;
					      
					      case 5:
						//Sair
					      break;
					      
					      case 6:
						//Listar
						enviar.control=6;
						if(send(s, &enviar, sizeof(struct mensagem), 0) < 0)
						{
						  perror("Send()");
						  exit(7);
						}
					      break;
					      case 7:
						//Listar
						enviar.control=7;
						if(send(s, &enviar, sizeof(struct mensagem), 0) < 0)
						{
						  perror("Send()");
						  exit(7);
						}
					      break;
					      default:
					      break;
	    		}

	     }while(1);

}
