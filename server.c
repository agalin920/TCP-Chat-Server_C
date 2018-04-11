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
    fd_set master;    //master file descriptor list
    fd_set read_fds;  //temp file descriptor list for select()
    int fdmax;        //max. amount of file descriptors

    int listener;     //listening socket descriptor
    int newfd;        //newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; //client address
    socklen_t addrlen;

    char buf[256];    //buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        //for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    //FD_ZERO works like memset 0;
    FD_ZERO(&read_fds);  //clear the master and temp sets

    // fetch a socket, bind it
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

    // main loop
    while(1)
    {
        read_fds = master; // Copiar
        select(fdmax+1, &read_fds, NULL, NULL, NULL);


        //run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) //EDIT HERE
        {
            if (FD_ISSET(i, &read_fds))
            {   // we got one
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master); // pass to master set
                        if (newfd > fdmax)
                        {   // keep track of the max
                            fdmax = newfd;
                        }
                        printf("Conexion nueva en Socket:%d\n",newfd);
                    }
                }
                else
                {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                    {   // got error or connection closed by client
                        if (nbytes == 0)
                        {   // connection closed
                            printf("Socket %d disconectado\n",i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else
                    {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++)
                        {
                            // broadcast to everyone
                            if (FD_ISSET(j, &master))
                            {
                                // except the listener and ourselves
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
                }//handle data from client
            } //new incoming connection
        } //looping through file descriptors
    } //for(;;)

    return 0;
}

