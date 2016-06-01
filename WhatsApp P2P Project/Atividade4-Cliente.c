#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAM_BUF 62*5+3
#define protocol_i 0
/*
 * Cliente TCP
 */

enum protocolo { cadastrar = 1, ler, apagar, ack, erro, invalido, querSair, vazio};

void sendResp(char s[TAM_BUF], int ns, int resp) {
    s[protocol_i] = resp;
    if (send(ns, s,1, 0) < 0)
        perror("Send()");
}   

int getsPutBuff(char buf[TAM_BUF], int j,int size) {
    char s[size+1];
    int i;
    fgets(s,size,stdin);
    for (i=0; s[i] ; i++) {
        if (s[i] == '\n') break;
        buf[j++] = s[i];
    }
    buf[j++] = '\0';
    return j;
}

int getNextString(char buf[TAM_BUF], int j) {
    while (buf[j++] != '\0');
    if (buf[j] == '#') return -1;
    return j;
}

int main(int argc,char *argv[]) {
    unsigned short port;       
    char sendbuf[TAM_BUF];              
    char recvbuf[TAM_BUF];               
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s, l;
    /*
     * O primeiro argumento (argv[1]) é o hostname do servidor.
     * O segundo argumento (argv[2]) é a porta do servidor.
     */
    if (argc != 3) {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }
    /*
     * Obtendo o endereço IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short) atoi(argv[2]);

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
    }

    int conectado = 1;
    do {
        int opcao;
        do {
            printf("\n**********************************\nOpcoes: \n1 - Cadastrar mensagem\n");
            printf("2 - Ler mensagens\n3 - Apagar mensagens\n4 - Sair da Aplicacao\n**********************************\n");
            scanf("%d",&opcao);
        } while (opcao < 1 || opcao > 4);
        switch (opcao) {
            case 1:
                getchar();
                sendbuf[protocol_i] = cadastrar;
                printf("Digite seu nome de usuario\n");
                l = getsPutBuff(sendbuf,1,10);        
                printf("Agora digite a mensagem\n");    
                l = getsPutBuff(sendbuf,l,50);
                if (send(s, sendbuf,l, 0) < 0) {
                    perror("Send()");
                    exit(5);
                }
                if (recv(s, recvbuf, sizeof(recvbuf), 0) == -1) {
                    perror("Recv()");
                    exit(6);
                }
                if (recvbuf[protocol_i] == ack)
                    printf("Mensagem cadastrada\n");
                else if (recvbuf[protocol_i] == erro)
                    printf("Erro. Limite de mensagens atingido\n");
                break;
            case 2:
                sendResp(sendbuf,s,ler);
                if (recv(s, recvbuf, sizeof(recvbuf), 0) == -1) {
                    perror("Recv()");
                    exit(6);
                }
                if (recvbuf[protocol_i] == vazio) {
                    printf("Nenhuma mensagem cadastrada no servidor.\n");
                } else if (recvbuf[protocol_i] == ack) {
                    l = 1;
                    printf("As seguintes mensagens estao cadastrada:\n");
                    do {
                        printf("  Nome: %-10s |  ",recvbuf+l);
                        l = getNextString(recvbuf,l);
                        if (l==-1) break;
                        printf("Mensagem: %s.\n",recvbuf+l);
                        l = getNextString(recvbuf,l);
                    } while (l!=-1);
                }
                break;
            case 3:
                getchar();
                sendbuf[protocol_i] = apagar;
                printf("Digite o nome do usuario que tera suas mensagens apagadas\n");
                l = getsPutBuff(sendbuf,1,10);
                if (send(s, sendbuf,l, 0) < 0) {
                    perror("Send()");
                    exit(5);
                }
                if (recv(s, recvbuf, sizeof(recvbuf), 0) == -1) {
                    perror("Recv()");
                    exit(6);
                }
                if (recvbuf[protocol_i] == vazio)
                    printf("Nenhuma mensagem cadastrada para este usuário\n");
                else if (recvbuf[protocol_i] == ack) {
                    l = 1;
                    int apagadas = 0;
                    do {
                         l = getNextString(recvbuf,l);
                         if (l==-1) break;
                         l = getNextString(recvbuf,l);
                         apagadas++;
                    } while (l!=-1);
                    l = 1;
                    printf("%d mensagens foram removidas. A seguir as mensagens removidas\n",apagadas);
                    do {
                        printf("  Nome: %-10s | ",recvbuf+l);
                        l = getNextString(recvbuf,l);
                        if (l==-1) break;
                        printf("Mensagem: %s.\n",recvbuf+l);
                        l = getNextString(recvbuf,l);
                    } while (l!=-1);
                }
                break;
            case 4:
                printf("Dispensando o servidor\n");
                sendResp(sendbuf,s,querSair);
                conectado = 0;
                printf("Saindo...\n");
                break;
        }
    } while (conectado);

    /* Fecha o socket */
    close(s);

    printf("Cliente terminou com sucesso.\n");
    return 0;
}


