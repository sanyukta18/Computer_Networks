#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
//#include<netinet/in.h> 
//#include<arpa/inet.h> 
#include<sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 100; i++)
        b[i] = '\0';
}

void main(int argc, char **argv)
{
  int fd;
  if(argc != 2)
  {
    printf("Usage: %s <port>\n", argv[0]);
    exit(0);
  }
  char filename[30];
   struct sockaddr_in si_other;
  //char buffer[100];//buffer1[100];
  socklen_t addr_size;
   

  int port = atoi(argv[1]);
  int sockfd;
  struct sockaddr_in serverAddr;
  char buffer[100]; char buffer1[100]; char content[10000];
 
  //socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&serverAddr, '\0', sizeof(serverAddr));

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  

  //strcpy(buffer, "Hello Server\n");
  printf("Enter filename with .txt: "); 
    scanf("%s",filename);
    printf("filename:%s\n",filename);

    fd = open(filename,O_RDONLY);
   // printf("fd: %d\n",fd); 
     if(fd<=0)
    {
      printf("No file found\n");
      return ;
    }
   
    printf("file opened\n");
    while(1)
    {
      read(fd,buffer,sizeof(buffer)-1);
      //printf("buffer: %ld\n",strlen(buffer));
      if(strlen(buffer) ==0)
      {
        sendto(sockfd, buffer, 100, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        break;
      }
      sendto(sockfd, buffer, 100, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
      clearBuf(buffer);
    }
    //strcpy(buffer,'!');
    //sendto(sockfd, buffer, 100, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    //printf("Data sent\n");
    clearBuf(buffer1);
    addr_size = sizeof(serverAddr);
    recvfrom(sockfd, buffer1, 100, 0,(struct sockaddr*)&serverAddr,&addr_size);
    printf("%s\n", buffer1);


}