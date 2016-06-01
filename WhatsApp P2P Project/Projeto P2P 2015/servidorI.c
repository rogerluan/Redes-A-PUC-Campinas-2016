/* Servidor.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>


struct usuarios *usuariosCad; 
struct arquivos arqs;       
struct arquivos *arquivos;  

struct mensagem
{ /* mensagem trocada entre cliente e servidor */
  int control;        /* LOGIN, RECEBER, BUSCA, LOGOUT */
  int flag;       
  char user[10];  
  char senha[14]; 
  char pChave[20];
  char data[50]
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

struct usuarios
{ 
  char user[10];
  char senha[14];
  struct usuarios *prox;
};

/* Ver se existe usuario e autentica */
int autenticar(struct usuarios **cadusers)
{

	  FILE *users;
	  struct usuarios *aux= usuariosCad;
	  struct usuarios *cad = malloc(sizeof(struct usuarios));
	  /* Verifica se o usuario ja esta cadastrado, e se ja estiver, se a senha esta correta ou nao */
	  while(1)
	  {
	    if(strcmp(enviar.user, aux->user) == 0)
	      if(strcmp(enviar.senha, aux->senha) == 0)
	        return 0;
	      else
	        return 1;
	    else
	    {
	      if(aux->prox != NULL)
	        aux = aux->prox;
	        else break;
	    }
	  }
  
	  /* Arquivo usado para gravar todo usuario que se cadastra */
	  users = fopen("usuarios.txt", "a");
	  if(users == NULL)
	  {
	    perror("Erro ao criar arquivo de usuarios!");
	    exit(1);
	  } 
   
    strcpy(cad->user, enviar.user);
    strcpy(cad->senha, enviar.senha);    
    cad->prox = (*cadusers);
    (*cadusers) = cad;
  


  fprintf(users, "%s\n%s\n", enviar.user, enviar.senha);
  fclose(users);
  return 2;
}

/* Retirar arquivos do usuario que está desconectando */
void logout()
{

}

/* Carregar usuarios cadastrados em outras execuções do servidor à partir de "./Server Files/usuarios.txt" */
void carregar_usuarios(struct usuarios **usuariosCad)
{
  FILE *users;
  int i=0;
  struct stat filestatus;
  struct usuarios *aux;
  
   
  if((users=fopen("usuarios.txt", "r")) == NULL)
  {
    perror("Erro ao abrir arquivo de usuarios!");
    exit(1);
  }
  stat("usuarios.txt", &filestatus);
  if(filestatus.st_size == 0)
  {
         return;
  }

  if(!feof(users))
  {
    
	    while(!feof(users))
	    {
	       aux = malloc(sizeof(struct usuarios));
	       fscanf(users, "%s\n%s\n", aux->user, aux->senha);
	      
	       aux->prox = (*usuariosCad);
	       (*usuariosCad) = aux;
	       i++;
	       __fpurge(stdin);
	    }
	    
  }
  fclose(users);
}


void carregar_arquivos(struct arquivos arqs, struct arquivos **arquivos)
{

      int i;
      char *t;
  
      struct arquivos *aux;
  
      fprintf(stdout,"Numero de arquivos = %d\n",arqs.numero);
  
      aux = malloc(sizeof(struct arquivos));
      aux->numero = arqs.numero;
      strcpy(aux->hostname, arqs.hostname);
      strcpy(aux->porta, arqs.porta);

      for(i = 0; i < arqs.numero; i++)
      {        
	      strcpy(aux->nome[i], arqs.nome[i]);
	      fprintf(stdout,"Nome do arquivo = %s",arqs.nome[i]);
	      strcpy(aux->desc[i], arqs.desc[i]);
      }

      aux->prox = (*arquivos);
      (*arquivos) = aux;

}

//procedimento para buscar palavras
void buscar(struct arquivos *arquivossss, int *ns)
{
  
  int i = 0;
  int j = 0;
  int k = 0;
  int flag = 0;
  struct arquivos * aux = arquivos;
  struct arquivos busca[20]; 
  busca[0].numero = 0;

  while(aux != NULL)
  {

	    for(i = 0; i < aux->numero; i++)
	    {
		      fprintf(stdout,"",enviar.pChave,aux->nome[0]);
		      if((strstr(aux->desc[i], enviar.pChave) != NULL) || (strstr(aux->nome[i], enviar.pChave) != NULL))
		      {
					strcpy(busca[k].hostname, aux->hostname);
					strcpy(busca[k].nome[j], aux->nome[i]);
					strcpy(busca[k].desc[j], aux->desc[i]);
					strcpy(busca[k].porta, aux->porta);
					busca[k].numero++;
					flag = 1; 
					j++;

					printf("Nome: %s", busca[k].nome[j]);
		      }
	    }
	    if(flag == 1)
	    {
	      k++;
	      busca[k].numero = 0;
	    }
	    aux = aux->prox;
	    
	    j = 0;
   }

  if(k == 0)
    strcpy(busca[0].hostname, "NADA");
  else
    strcpy(busca[k].hostname, "NADA");
  

	  if(send(ns, &busca, (sizeof(struct arquivos) * 20), 0) < 0)
	  {
	    perror("Send()");
	    exit(7);
	  }

}

void listar_usuarios(struct usuarios *usuariosCadEx)
{   
   struct usuarios * aux = usuariosCad;
   while(aux != NULL)
   {     
       printf("Usuario: %s\nSenha: %s\n\n", aux->user, aux->senha);
       aux=aux->prox;
   }
  
}

void listar_arquivos(struct usuarios *usuariosCadEx)
{      
   struct arquivos * aux = arquivos;
   while(aux != NULL)
   {    
       printf("Nome Arquivo: %s\n Descricao: %s\n", aux->nome[0], aux->desc[0]);
       aux=aux->prox;
   }
  
}

int receber(int *nns)
{	
      __fpurge(stdin);
     //Abre o arquivo binario requisitado com opcção de escrita*/
      FILE *fp = fopen(enviar.data,"r");

      if(fp==NULL)
      {
        perror("ERRO AO ABRIR ARQUIVO \n");
        return 1;   
      }

      struct sockaddr_in serv_addr;
      char sendBuff[1022];
      int numrv;

      while(1)
      {
      
        //Lê o arquivo binario com opção de apenas leitura

        unsigned char buff[256]={0};
        int nbytes = fread(buff,6,256,fp);
        printf("Buff:%s \n", buff);
        printf("Bytes read %d \n", nbytes);
    

        memcpy(enviar.data,buff,strlen(buff)+1);   
   


  	    //Manda o conteudo do buffer para o cliente
        if(nbytes > 0)
        {
          printf("Enviando \n");
          printf("Conteudo:%s\n",buff );
          write(nns, buff, nbytes);
        }

        /*Verifica se ocorreu erro de leitura */
        if (nbytes < 256)
        {
          if (feof(fp))
            fprintf(stderr,"Fim do Arquivo\n");

          if (ferror(fp))
            fprintf(stderr,"Erro de leitura\n");

          break;
        }

      }
      
     //Manda o conteudo do buffer para o cliente
     
     if (send(nns, &enviar, sizeof(struct mensagem), 0) < 0)
     {
        perror("sendto()");
        exit(2);
     }
     
     fprintf(stdout,"Enviado\n");

    fclose(fp);
      

    }


void *function(void *nss)
{
  int ns = *(int *)nss;

  do{
   
		    if(recv(ns, &enviar, sizeof(struct mensagem), 0) == -1)
		    {
		      perror("Recv()");
		      exit(6);
		    }
		    if(enviar.control == 0)
		    {
		      	  enviar.flag = autenticar(&usuariosCad);

			      if(send(ns, &enviar, sizeof(struct mensagem), 0) < 0)
			      {
			        perror("Send()");
			        exit(7);
			      }
		    }
	        else if(enviar.control == 1)
	        {
	            enviar.flag = 0;
	        }
	        
	        else if(enviar.control == 2)
	        {
	      		buscar(arquivos, ns);
	      		enviar.flag=9;
	        }
		    else if(enviar.control == 3)
		    {
		      //enviar
		    }  
		    else if(enviar.control == 4)
		    {
		        
		          logout();
		    }
		    else if(enviar.control == 5)
		    {
		    
		      //sair
		    }
		    else if(enviar.control == 6)
		    {
				listar_usuarios(&usuariosCad);
				__fpurge(stdin);
		    }
		    else if(enviar.control == 7)
		    {
				listar_arquivos(&usuariosCad);
				__fpurge(stdin);
		    }

		    else if(enviar.control == 40)
		    {
			       receber(ns);
    		    }


		    if(enviar.flag == 2 || enviar.flag == 0)
		    {
			      if(recv(ns, &arqs, sizeof(struct arquivos), 0) == -1)
			      {
			        perror("Recv()");
			        exit(6);
			      }
			      carregar_arquivos(arqs, &arquivos);
			      // enviar.control = 9;
			      enviar.flag = 1000;
		    }

    }while(1);
}

/*
 * Servidor TCP
 */
int main(int argc, char **argv)
{
 
	  int s; /* Socket para aceitar conexões       */
	  int namelen;
	  int at; // retorno da funcao autenticar
	  int nthread = 0; /* numero de clientes conectados */	
	  int ns;
	  unsigned short port;
	  struct sockaddr_in server; 
	  struct sockaddr_in client;
	  pthread_t threads[10];

	  usuariosCad = malloc(sizeof(struct usuarios));
	  usuariosCad->prox = NULL;


	  port = atoi(argv[1]);

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

	  /*
	   * Aceita uma conexão e cria um novo socket através do qual
	   * ocorrerá a comunicação com o cliente.
	   */
	  
	  namelen = sizeof(client);
	  carregar_usuarios(&usuariosCad);

	  do{
		    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
		    {
		      perror("Accept()");
		      exit(5);
		    }

		    pthread_create(&threads[nthread], NULL, function, (void *)&ns);
		    nthread++;
	    }while(1);

}




