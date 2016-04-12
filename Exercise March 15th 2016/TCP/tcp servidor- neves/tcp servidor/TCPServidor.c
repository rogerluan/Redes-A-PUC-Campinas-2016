//
//  main.c
//  tcp servidor
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
    int sockint,s, namelen,ns;
    struct sockaddr_in client, server;
    struct mensagem buf;
    struct mensagem vetor_mensagens[5];
    struct mensagem aux_vetor_mensagens[5];
    struct mensagem deletadas_vetor_mensagens[5];

    int quantidadeMensagens = 0;

    
    /*
     * Cria um socket TCP (stream) para aguardar conexıes
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
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
        exit(3);
    }
    
    /*
     * Prepara o socket para aguardar por conexıes e
     * cria uma fila de conexıes pendentes.
     */
    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    
    
    /*
     * Aceita uma conex„o e cria um novo socket atravÈs do qual
     * ocorrer· a comunicaÁ„o com o cliente.
     */
    namelen = sizeof(client);
    if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
    {
        perror("Accept()");
        exit(5);
    }
    
    while (1) {
        if(recv(ns, &buf, sizeof(buf), 0) == -1)
        {
            perror("recvfrom()");
            exit(6);
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
            
            
            
            
            if (send(ns, &buf, sizeof(buf), 0) < 0)
            {
                perror("sendto()");
                exit(7);
            }
            
        }
        
        if (buf.codigo == 2) {
            vetor_mensagens[0].quantidade = quantidadeMensagens;
            
            
            if (send(ns, &vetor_mensagens, sizeof(vetor_mensagens), 0) < 0)
            {
                perror("sendto()");
                exit(7);
            }
            
            
        }
        
        if(buf.codigo == 3){
            int aux = quantidadeMensagens;
            int auxQuantidaDeMensagens = 0;
            int mensagensDeletadas = 0;
            //passa pelas mensagens colocando mensagens de usuarios corretos em auxiliar e deletados no de
            //deletados
            for (int i = 0; i < quantidadeMensagens; i++) {
                
                //usuario detectado
                if (strcmp(vetor_mensagens[i].usuario, buf.usuario) == 0) {
                    deletadas_vetor_mensagens[mensagensDeletadas] = vetor_mensagens[i];
                    mensagensDeletadas++;
                    
                    
                    
                }else{      //usuario que nao precisa ser deletado
                    
                    aux_vetor_mensagens[auxQuantidaDeMensagens] = vetor_mensagens[i];
                    auxQuantidaDeMensagens++;
                    
                    
                }
            }
            quantidadeMensagens = auxQuantidaDeMensagens;
            
            for (int i = 0; i < quantidadeMensagens; i++) {
                
                vetor_mensagens[i] =aux_vetor_mensagens[i];
                
            }
            vetor_mensagens[0].quantidade = quantidadeMensagens;
            deletadas_vetor_mensagens[0].quantidade = mensagensDeletadas;
            
            if (send(ns, &deletadas_vetor_mensagens, sizeof(deletadas_vetor_mensagens), 0) < 0)
            {
                perror("sendto()");
                exit(7);
            }
            

        }
        
        
        
        
    }
    
    
    
    /*
     * Fecha o socket.
     */
    close(s);
}
