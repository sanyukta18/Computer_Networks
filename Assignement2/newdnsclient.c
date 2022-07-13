#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define SERVER_PORT 9009
#define CLIENT_PORT 8992 
#define MAXLINE 1000

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 1024; i++)
        b[i] = '\0';
}


int main()
{
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    // Configure the server and current address
    struct sockaddr_in server, client;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(CLIENT_PORT);

    // Bind the socket to the address
    if( bind(sockfd, (const struct sockaddr *)&client, sizeof(client)) < 0)
    {
        printf("Bind failure\n");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if( connect(sockfd, (const struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Cannot establish connection\n");
        exit(EXIT_FAILURE);
    }
      printf("Enter the DNS: ");
    char send_b[MAXLINE], recv_b[MAXLINE];
    scanf("%[^\n]",send_b);
    printf("DNS is: %s\n",send_b);
    // Send the domain to the server
    send(sockfd, send_b, sizeof(send_b), 0);
    char buffer1[1000];
    clearBuf(buffer1);
   printf("Addresses are: \n");
    while(1)
    {
        //socklen_t addr_size;
        //addr_size = sizeof(client);
         
        recv(sockfd, buffer1, 1000, 0);
        if(strlen(buffer1) == 0)
            break;
        printf("%s\n",buffer1);
        clearBuf(buffer1);
    }
   close(sockfd);
  printf("Disconnected from the server.\n");
    return 0;
}