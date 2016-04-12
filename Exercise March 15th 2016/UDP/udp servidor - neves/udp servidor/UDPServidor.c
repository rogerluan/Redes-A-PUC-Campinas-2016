//
//  UDPServidor.c
//  udp servidor
//
//  Created by Gabriel Neves Ferreira.
//  Copyright © 2016 Neves. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


struct mensagem{
    int codigo;
    char usuario[10];
    char mensagem[50];
    char resposta[50];
    int quantidade;

    
};


/*
 * Servidor UDP
 */
main()
{
    int sockint,s, namelen, client_address_size;
    struct sockaddr_in client, server;
    struct mensagem buf;
    struct mensagem vetor_mensagens[5];
    int quantidadeMensagens = 0;
    /*
     * Cria um socket UDP (dgram).
     */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket()");
        exit(1);
    }
    
    /*
     * Define a qual endereÁo IP e porta o servidor estar· ligado.
     * Porta = 0 -> faz com que seja utilizada uma porta qualquer livre.
     * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
     * os endereÁos IP
     */
    server.sin_family      = AF_INET;   /* Tipo do endereÁo             */
    server.sin_port        = htons(5000);         /* Escolhe uma porta disponÌvel */
    server.sin_addr.s_addr = INADDR_ANY;/* EndereÁo IP do servidor      */
    
    /*
     * Liga o servidor ‡ porta definida anteriormente.
     */
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind()");
        exit(1);
    }
    
    /* Consulta qual porta foi utilizada. */
    namelen = sizeof(server);
    if (getsockname(s, (struct sockaddr *) &server, &namelen) < 0)
    {
        perror("getsockname()");
        exit(1);
    }
    //ate este momento definicoes do servidor
    
    
    
    
    while (1) {
        client_address_size = sizeof(client);
        if(recvfrom(s, &buf, sizeof(buf), 0, (struct sockaddr *) &client,
                    &client_address_size) <0)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        if(buf.codigo == 1){
            
            if(quantidadeMensagens < 5){
                
                strcpy(vetor_mensagens[quantidadeMensagens].usuario, buf.usuario);
                strcpy(vetor_mensagens[quantidadeMensagens].mensagem, buf.mensagem);
                quantidadeMensagens++;
                strcpy(buf.resposta, "mensagem recebida");
                
            }else{
                
                strcpy(buf.resposta, "armazenamento lotado");

                
            }

            
            
            
            if (sendto(s, &buf, sizeof(buf), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(2);
            }
            
        }
        
        if (buf.codigo == 2) {
            vetor_mensagens[0].quantidade = quantidadeMensagens;
            
            
            if (sendto(s, &vetor_mensagens, sizeof(vetor_mensagens), 0, (struct sockaddr *)&client, sizeof(client)) < 0)
            {
                perror("sendto()");
                exit(2);
            }

            
        }
       
        
        
        
    }
    
    
    /*
     * Fecha o socket.
     */
    close(s);
}




