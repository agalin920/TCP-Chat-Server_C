#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9074"   // Puerto


int main(void)
{
    fd_set master;    //lista descriptor maestro
    fd_set read_fds;  //temp list para select
    int fdmax;        //cantidad max de fd
    int yes=1;   
    int listener;     
    int newfd;        //descriptor nuevo (socket)
    struct sockaddr_storage remoteaddr; //Dirrecion cliente
    socklen_t addrlen;
    char buf[100];    //buffer for client data
    int nbytes;
    int i, j;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    //memset 0
    FD_ZERO(&read_fds);  //resetear sets

    // Bind un socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PORT, &hints, &ai);

    //Loop resultados
    for(p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    printf("Servidor inicializado\n");
    listen(listener, 10);
    printf("Servidor esuchando...\n");

    // Agregar listener al set master
    FD_SET(listener, &master);

    // Trackear descriptor mas grande
    fdmax = listener;

    // Loop infinito
    while(1)
    {
        read_fds = master; // Copiar
        select(fdmax+1, &read_fds, NULL, NULL, NULL);


        //Loop atravez conexiones para encontrar mensajes
        for(i = 0; i <= fdmax; i++) 
        {
            if (FD_ISSET(i, &read_fds))
            {  
                if (i == listener)
                {
                    // Nueva conexion
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                   
                        FD_SET(newfd, &master); // Paar a master set
                        if (newfd > fdmax)
                        {   // Trackear max
                            fdmax = newfd;
                        }
                        printf("Conexion nueva en Socket:%d\n",newfd);
                    
                }
                else
                {
                    // Recibir mensaje del cliente
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                    {   
                        if (nbytes == 0)
                        {   // Conexion terminada
                            printf("Socket %d disconectado\n",i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i); // Cerrar Socket
                        FD_CLR(i, &master); // Remover del master set
                    }
                    else
                    {
                        // Hay mensaje
                        for(j = 0; j <= fdmax; j++)
                        {
                            // Hacer el broadcast
                            if (FD_ISSET(j, &master))
                            {
                                // Menos al listener y al server
                                if (j != listener && j != i)
                                {
                                    if (send(j, buf, nbytes, 0) == -1)
                                    {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                }
            } 
        } 
    } 

    return 0;
}
