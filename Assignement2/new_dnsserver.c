#include<stdio.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define SERVER_PORT 9009
#define MAXLINE 1001


void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 1000; i++)
        b[i] = '\0';
}
int main()
{
    int i;
    // Create sockets for both protocols and check for failure
    int sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0),
            sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd_udp < 0 | sockfd_tcp < 0)
    {
        printf("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    struct sockaddr_in server, client;
    socklen_t size=sizeof(client);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    // Bind the sockets to the address
    if( bind(sockfd_udp, (const struct sockaddr *)&server, sizeof(server)) < 0 || bind(sockfd_tcp, (const struct sockaddr *)&server, sizeof(server)) < 0 )
    {
        printf("Bind failure\n");
        exit(EXIT_FAILURE);
    }

    // Listen for TCP connections
    listen(sockfd_tcp, 10);

    // Maximum file descriptor
    int maxfd = sockfd_tcp < sockfd_udp ? sockfd_udp : sockfd_tcp;

    // Initialise fd_set
    fd_set fdSet;
    FD_ZERO(&fdSet);

    while(1)
    {
        // Include the sockets for tcp and udp in select
        FD_SET(sockfd_tcp, &fdSet), FD_SET(sockfd_udp, &fdSet);
        
        // Wait for socket ready to be read
        select(maxfd+1, &fdSet, NULL, NULL, NULL);

        // Handle new tcp connection
        if(FD_ISSET(sockfd_tcp, &fdSet))
        {

            struct sockaddr_in client;
            socklen_t size=sizeof(client);
            char send_b[MAXLINE], recv_b[MAXLINE];
            
            // Handle the new connectionn
            if(fork() == 0)
            {
                // Accept the new connection

                int new_sockfd = accept(sockfd_tcp, (struct sockaddr *)&client, &size);

                printf("Established connection with client %s:%d\n",
                            inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                char buffer[MAXLINE];
                clearBuf(buffer);
                recv(new_sockfd, buffer, sizeof(buffer), 0);
                // Retrieve the IP for the domain
               // printf("DOmain is %s\n",buffer);
                struct hostent *h = gethostbyname(buffer);
                struct in_addr **addr_lists;
                addr_lists = (struct in_addr **)h->h_addr_list;
                /////
                     // Convert the ip to the required format
                int j = 0;
                 char send_a[MAXLINE];
                if(h == NULL)
                    strcpy(send_a, "0.0.0.0");
                else
                {
                     //  send(new_sockfd,send_a, 1000, 0);
                    while(1)
                    {
                        strcpy(send_a,"");
                        
                        if(addr_lists[j]!= NULL)
                        {
                            strcpy(send_a,inet_ntoa(*addr_lists[j]));
                            send(new_sockfd,send_a,1000,0);
                        }
                      // printf("strings are%s\n",send_a);
                       j++;
                       if(send_a == "")
                        {
                            send(new_sockfd,send_a,1000,0);
                            return 0;
                        }
                       clearBuf(send_a);
                       
                        //strcpy(send_b,inet_ntoa(*(struct in_addr *)(he->h_addr_list[0])));
                    }
                }

                // Close socket
                close(new_sockfd); 
            }
        }

        // Handle new UDP connection
        else
        {
            struct sockaddr_in client;
            socklen_t size=sizeof(client);
            char send_b[MAXLINE], recv_b[MAXLINE];

            // Receive request from dnsclient
            recvfrom(sockfd_udp, recv_b, sizeof(recv_b), 0,(struct sockaddr *)&client, &size);
            struct in_addr **addr_list;
            struct in_addr addr;

            // Retrieve the IP for the domain
             struct hostent *he = gethostbyname(recv_b);
             addr_list = (struct in_addr **)he->h_addr_list;
            // Convert the ip to the required format
            if(he == NULL)
                strcpy(send_b, "0.0.0.0");

            else
            {
                for(i = 0; addr_list[i] != NULL; i++) 
              {
                strcpy(send_b,inet_ntoa(*addr_list[i]));
                sendto(sockfd_udp, send_b, 1000, 0, (struct sockaddr*)&client, sizeof(client));
                clearBuf(send_b);
              }
              strcpy(send_b," ");
              sendto(sockfd_udp,send_b, 1000, 0, (struct sockaddr*)&client, sizeof(client));
                //strcpy(send_b,inet_ntoa(*(struct in_addr *)(he->h_addr_list[0])));
            }

            // Send it back to the client
            //sendto(sockfd_udp, send_b, sizeof(send_b), 0,(const struct sockaddr *)&client, size);
        }
    }
    close(sockfd_udp);
    close(sockfd_tcp);
}