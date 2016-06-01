#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * Cliente UDP
 */


#define TAM 8195

struct msg{
	char msg[52];
	char usr_name[12];
};



struct udpBuffer
{
  char buffer[TAM];
  int pos;
};





unsigned char readByte(struct udpBuffer *buff)
{
  unsigned char retval = buff->buffer[buff->pos];
  buff->pos++;
  return retval;
}

int readInt(struct udpBuffer *buff)

{

  int retval = (*(int*)(&buff->buffer[buff->pos]));

  buff->pos+=4;

  return retval;

}

char *readString(struct udpBuffer *buff)
{
  char now;
  int initialPos=buff->pos;
  int size;
  char *retval;
  
  now = buff->buffer[buff->pos];
  while(now != '\0' && buff->pos < 8195)
  {
    buff->pos++;
    now = buff->buffer[buff->pos];
  }
  
  size = buff->pos-initialPos;
  retval = (char*)malloc(size);
  memcpy(retval,&buff->buffer[initialPos],size);
  buff->pos++;
  return retval;
}


void clearBuffer(struct udpBuffer *buff)
{
  buff->pos = 0;
}

void buffWrite(void *ptr, int size,struct udpBuffer *buff)
{
  memcpy(&buff->buffer[buff->pos],ptr,size);
  buff->pos+=size;
}

void writeInt(int value,struct udpBuffer *buff)
{
  buffWrite(&value, 4,buff);
}

void writeByte(unsigned char byte, struct udpBuffer *buff)
{
  buff->buffer[buff->pos] = byte;
  //memcpy(&buff->buffer[buff->pos],&byte,1);
  buff->pos++;
}

void writeString(char *string, struct udpBuffer *buff)
{

  int size = 0;
  int strChar = string[0];
  while(strChar!='\0' && size<1000)
  {
    size++;
    strChar = string[size];
  }
  size++;
  
  memcpy(&buff->buffer[buff->pos],string,size);
  buff->pos+=size;
}


main(argc, argv)
int argc;
char **argv;
{
    unsigned short port;       
    char sendbuf[12];              
    char recvbuf[12];              
    struct sockaddr_in client; 
    struct sockaddr_in server; 
    int s;                     /* Socket para aceitar conexões       */
    int ns;                    /* Socket conectado ao cliente        */
    int namelen;               

    /*
     * O primeiro argumento (argv[1]) é a porta
     * onde o servidor aguardará por conexões
     */
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s porta\n", argv[0]);
        exit(1);
    }

    port = (unsigned short) atoi(argv[1]);

    /*
     * Cria um socket TCP (stream) para aguardar conexões
     */
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


}
