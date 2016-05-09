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
#include <stdbool.h>


struct mensagem{
    char ra[9];
    bool isDoor;
    bool isPresent;
    
    
};




int main()
{
    int s, namelen,ns;
    struct sockaddr_in client, server;
    struct mensagem buf;
    char bufferer[33];

    int doorCounter = 0,presenceCounter = 0;
    char alunosRA[300][9];
    

    
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
    server.sin_port        = htons(5001);         /* Escolhe uma porta disponÌvel */
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
    //servidor fica esperando conexoes tcps, recebe uma mensagem e depois encerra ela, para que seja capaz
    //de receber outra conexao.
   // printf("pronto para mais");

  
    while (1) {

        if (listen(s, 1) != 0)
        {
            perror("Listen()");
            exit(4);
        }
        //aceita uma conexao
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        if(recv(ns, &bufferer, sizeof(bufferer), 0) == -1)
        {
            perror("recvfrom()");
            exit(6);
        }
        
        
        printf("recebi\n");

        if(bufferer[19] == '0'){
            buf.isDoor = false;
        }else{
            buf.isDoor = true;
        }

        if(bufferer[31] == '0'){
            buf.isPresent = false;
        }else{
            buf.isPresent = true;
        }
        
        
        for(int i = 0;i < 8;i++){
            
            buf.ra[i] = bufferer[3 + i];
            
        }
        
        
       // printf("eita");
      
        
        //identificacao de alguem que passou na porta
        if(buf.isDoor){
           // printf("contando");
            doorCounter++;
            
        }else{
            //requisicao do aplicativo,inicialmente checa se o RA ja se encontra no sistema
            
            if(buf.isPresent){
                
                //printf("aluno");
                
                int flag = 0;
                
                for (int i = 0; i < presenceCounter; i++) {
                
                    char temp[9];
                    strcpy(temp, buf.ra);
                    temp[8] = '\0';
                    
                    //igual
                    if(strcmp(alunosRA[i], buf.ra) == 0){
                        flag++;
                       // printf("repetido");
                    }
                    
                }
                
                if(!flag){
                    strcpy(alunosRA[presenceCounter], buf.ra);
                    alunosRA[presenceCounter][8] = '\0';
                    presenceCounter++;


                    
                }
            }
            
        }
         fflush(stdout);
        
        printf("\n\n----------\n\n");
        
        for (int i  = 0; i < presenceCounter; i++) {
            printf("%s\n",alunosRA[i]);
            
        }
        
        
    
        //fecha o socket, pois apenas será recebido uma mensagem por conexao
       // close(s);
    
    }
  
    
}





