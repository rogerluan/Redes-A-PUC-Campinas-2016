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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


struct mensagem{
    char ra[9];
    bool isDoor;
    bool isPresent;
    
    
};


struct data{
    
    char alunosRA[300][9];
    int count;
    int doorCounter
};



int main()
{
    int s, namelen,ns;
    struct sockaddr_in client, server;
    struct mensagem buf;
    char bufferer[33];

    
    //memoria = mmap(NULL, sizeof *memoria, PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    

    size_t shmsize = sizeof(struct data);
    int shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0666);
    struct data *data = shmat(shmid, (void *) 0, 0);
    data->count = 0;
    data->doorCounter = 0;
    
    
    
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
    

    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
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
    //printf("pronto para mais");

    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }
    

    //servidor fica esperando conexoes tcps, recebe uma conexao abre um fork
    //variaveis para fork
    pid_t pid, fid;
  
    while (1) {

 
        //aceita uma conexao
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
        {
            //perror("Accept()");
            
            //exit(5);
        }
        
        
        if ((pid = fork()) == 0) {
            /*
             * Processo filho
             */
            
            /* Fecha o socket aguardando por conexıes */
            close(s);
            
            /* Processo filho obtem seu prÛprio pid */
            fid = getpid();
            
            /* Recebe uma mensagem do cliente atravÈs do novo socket conectado */
         
            
            
            if(recv(ns, &bufferer, sizeof(bufferer), 0) == -1)
            {
                //perror("recvfrom()");
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
            
            
            
            
            //identificacao de alguem que passou na porta
            if(buf.isDoor){
                // printf("contando");
                data->doorCounter++;
                
            }else{
                //requisicao do aplicativo,inicialmente checa se o RA ja se encontra no sistema
                
                if(buf.isPresent){
                    
                    //printf("aluno");
                    
                    int flag = 0;
                    
                    for (int i = 0; i < data->count; i++) {
                        
                        char temp[9];
                        strcpy(temp, buf.ra);
                        temp[8] = '\0';
                        
                        //igual
                        if(strcmp(data->alunosRA[i], buf.ra) == 0){
                            flag++;
                            // printf("repetido");
                            
                            //enviar mensagem falando que nao foi aceitado
                            char resp[22];
                            resp[0] = 'R';
                            resp[1] = 'A';
                            resp[2] = ':';
                            resp[3] = temp[0];
                            resp[4] = temp[1];
                            resp[5] = temp[2];
                            resp[6] = temp[3];
                            resp[7] = temp[4];
                            resp[8] = temp[5];
                            resp[9] = temp[6];
                            resp[10] = temp[7];
                            resp[11] = ',';
                            resp[12] = 'r';
                            resp[13] = 'e';
                            resp[14] = 'c';
                            resp[15] = 'e';
                            resp[16] = 'b';
                            resp[17] = 'e';
                            resp[18] = 'u';
                            resp[19] = ':';
                            resp[20] = '0';
                            resp[21] = '\0';

                            //comentado aguardando cliente receber
                            /*
                            if (send(ns, resp, strlen(resp)+1, 0) < 0)
                            {
                                perror("Send()");
                                exit(7);
                            }
                            */
                            
                            
                        }
                        
                    }
                    
                    if(!flag){
                        strcpy(data->alunosRA[data->count], buf.ra);
                        data->alunosRA[data->count][8] = '\0';
                        data->count++;
                        
                        //enviar mensagem falando que foi aceitado
                        
                        char resp[22];
                        resp[0] = 'R';
                        resp[1] = 'A';
                        resp[2] = ':';
                        resp[3] = buf.ra[0];
                        resp[4] = buf.ra[1];
                        resp[5] = buf.ra[2];
                        resp[6] = buf.ra[3];
                        resp[7] = buf.ra[4];
                        resp[8] = buf.ra[5];
                        resp[9] = buf.ra[6];
                        resp[10] = buf.ra[7];
                        resp[11] = ',';
                        resp[12] = 'r';
                        resp[13] = 'e';
                        resp[14] = 'c';
                        resp[15] = 'e';
                        resp[16] = 'b';
                        resp[17] = 'e';
                        resp[18] = 'u';
                        resp[19] = ':';
                        resp[20] = '1';
                        resp[21] = '\0';
                        
                        //comentado aguardando cliente receber
                        /*
                        if (send(ns, resp, strlen(resp)+1, 0) < 0)
                        {
                            perror("Send()");
                            exit(7);
                        }
                        
                        */
                        
                        
                        
                        
                    }
                }
                
            }
            fflush(stdout);
            
            printf("\n\n----------\n\n");
            printf("entraram %d pessoas\n----------\nRAs cadastrados:\n",data->doorCounter);
            
            for (int i  = 0; i < data->count; i++) {
                printf("%s\n",data->alunosRA[i]);
                
            }
            
            

            
            
            
            
            
            /* Fecha o socket conectado ao cliente */
            close(ns);
            
            /* Processo filho termina sua execuÁ„o */
            printf("[%d] Processo filho terminado com sucesso.\n", fid);
            exit(0);
        }
        else
        {
            
            
            
            /*
             * Processo pai 
             */
            if (pid > 0)
            {
                //printf("Processo filho criado: %d\n", pid);
                
                /* Fecha o socket conectado ao cliente */
                close(ns);
                while (waitpid(0,NULL,WNOHANG)!=-1); // clean system tables non blocking

            }
            else
            {
                close (ns);
                while (waitpid(0,NULL,WNOHANG)!=-1); // clean system tables non blocking
                //perror("Fork()");
                //exit(7);
            }
        }
        
        
        
        
        
        
       
    
    }
  
    
}





