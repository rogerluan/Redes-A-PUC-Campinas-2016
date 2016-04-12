//
//  UDPCliente.c
//  udp cliente
//
//  Created by Gabriel Neves Ferreira.
//  Copyright © 2016 Neves. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct mensagem{
    int codigo;
    char usuario[10];
    char mensagem[50];
    char resposta[50];
    int quantidade;
    
};



/*
 * Cliente UDP
 */
main(argc, argv)
int argc;
char **argv;
{
    
    
    int s;
    unsigned short port;
    struct sockaddr_in server;
    struct mensagem buf;
    struct mensagem vetor_mensagens[5];
    
    /*
     * O primeiro argumento (argv[1]) È o endereÁo IP do servidor.
     * O segundo argumento (argv[2]) È a porta do servidor.
     */
    
    argv[1] = "192.168.1.34";
    argv[2] = "5000";
    
    port = htons(atoi(argv[2]));
    
    /*
     * Cria um socket UDP (dgram).
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }
    
    /* Define o endereÁo IP e a porta do servidor */
    server.sin_family      = AF_INET;            /* Tipo do endereÁo         */
    server.sin_port        = port;               /* Porta do servidor        */
    server.sin_addr.s_addr = inet_addr(argv[1]); /* EndereÁo IP do servidor  */
    
   
    
    //ate este momento definicoes do cliente
    
    
    int cod = 0;
    while(cod != 3){
    
        
        printf("\n---------\ndigite:\n1 para enviar mensagem\n2 ler mensagens\n3 para sair da aplicacao");
        scanf("%d",&cod);
        if(cod == 3){
            break;
            
        }
        if(cod == 1){
            buf.codigo = cod;
            printf("digite usuario\n");
            scanf("%s",buf.usuario);
            printf("digite mensagem\n");
            scanf("%s",buf.mensagem);
            
            if (sendto(s, &buf, sizeof(buf), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                perror("sendto()");
                exit(2);
            }
            
            int server_address_size = sizeof(server);
            if(recvfrom(s, &buf, sizeof(buf), 0, (struct sockaddr *) &server,&server_address_size) <0)
            {
                perror("recvfrom()");
                exit(1);
            }
            
            printf("\n%s\n",buf.resposta);

            
        }
        if(cod == 2){
            buf.codigo = cod;
            //manda apenas o codigo
            if (sendto(s, &buf, sizeof(buf), 0, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                perror("sendto()");
                exit(2);
            }
            //retorna o vetor armazenado
            int server_address_size = sizeof(server);
            if(recvfrom(s, &vetor_mensagens, sizeof(vetor_mensagens), 0, (struct sockaddr *) &server,&server_address_size) <0)
            {
                perror("recvfrom()");
                exit(1);
            }
            //checa quantas mensagens possui
            int count = vetor_mensagens[0].quantidade;
            if(count == 0){
                
                printf("nao possui mensagens");
                
                
            }else{
                printf("mensagens cadastradas: %d",count);
                //print mensagens
                for(int i = 0;i < count; i++){
                
                    printf("--\nusuario: %s\nmensagem: %s\n",vetor_mensagens[i].usuario,vetor_mensagens[i].mensagem);
                }
            }

            
            
            
        }
        
    
    }
    
 
    
    printf("\n----programa finalizado----\n");
    
    /* Fecha o socket */
    close(s);
}



