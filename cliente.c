#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT "9074" //Puerto


void *recibir(void *);


int main(int argc, char *argv[])
{
    char mensaje[100];
    char nick[30];
    char ip[30];
    char port[50];
    int sockfd;
    char bufferMensaje[100];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&nick, sizeof(nick), 0);
    memset(&mensaje, sizeof(mensaje), 0);
    memset(&ip, sizeof(mensaje), 0);
    memset(&hints, 0, sizeof hints);

    printf("IP del servidor: ");
    fgets(ip, 30, stdin);;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, PORT, &hints, &servinfo);

    // loop todos los resultados
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }
    
    //Thread para recibir mensajes
    pthread_t recibir_thread;
    pthread_create(&recibir_thread, NULL, recibir, (void*)(intptr_t) sockfd);

    //Obtener Nick
    printf("Nick: ");
    fgets(nick, 30, stdin);  
      
    printf("Conectado al servidor Minichat\n");
    printf("Escribe 'exit' para salirte\n\n\n");

    
    while(1)
    {
        char temp[6];
        memset(&temp, sizeof(temp), 0);

        memset(&bufferMensaje, sizeof(bufferMensaje), 0); //clean sendBuffer
        fgets(bufferMensaje, 100, stdin); //gets(message);

        if(bufferMensaje[0] == 'e' && bufferMensaje[1] == 'x' && bufferMensaje[2] == 'i' && bufferMensaje[3] == 't')
            return 1;
        
        //Agregar nick al mensaje   
        int count = 0;
        while(count < strlen(nick))
        {
            mensaje[count] = nick[count];
            count++;
        }
        count--;
        mensaje[count] = '>';
        count++;
        

        for(int i = 0; i < strlen(bufferMensaje); i++)
        {
            mensaje[count] = bufferMensaje[i];
            count++;
        }
        mensaje[count] = '\0';
        
        //Imprimir mensaje de nuevo con nick
        //puts(message);
        
        //Mandar mensaje
        send(sockfd, mensaje, strlen(mensaje), 0); 
        
        //Resetear buffer
        memset(&bufferMensaje, sizeof(bufferMensaje), 0);
        
    }
    
    //puts("Closing socket connection");
    pthread_join(recibir_thread , NULL);
    close(sockfd);

    return 0;
}

//Funcion para thread
void *recibir(void *sock_fd)
{
    int sfd = (intptr_t) sock_fd;
    char buffer[100];
    int numBytes;
    
    while(1)
    {
        numBytes = recv(sfd, buffer, 100-1, 0);
        buffer[numBytes] = '\0';
        printf("%s", buffer);
    }
}
