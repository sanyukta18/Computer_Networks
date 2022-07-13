#include<stdlib.h> 
#include<string.h> 
#include<stdio.h> 
#include<sys/types.h> 
#include<sys/socket.h> 
#include <sys/stat.h>
#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

#define MAX_INPUT_SIZE 100

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 100; i++)
        b[i] = '\0';
}

int main()
{

  char *ip = "127.0.0.1";
  int port = 5580;

  int sock,fd;
  struct sockaddr_in addr;
  socklen_t addr_size;
  char buffer[100]; char filename[30];
  int n;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = inet_addr(ip);

  connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  printf("Connected to the server.\n");


  printf("Enter filename as filename.txt: "); 
    scanf("%s",filename);
    printf("filename:%s\n",filename);
    fd = open(filename,O_RDONLY);
    //printf("fd: %d\n",fd);  
    if(fd<=0)
    {
      printf("No file found\n");
      return 0;
    }
    printf("file opened\n");
    //send_file(fd,sock);
 
  lseek(fd,0,SEEK_SET);
  while(1)
  {
    read(fd, buffer, sizeof(buffer));
    //printf("%ld\n",strlen(buffer));
    if(strlen(buffer) == 0)
    {
        sendto(sock, buffer, 100, 0, (struct sockaddr*)&addr, sizeof(addr));
        break;
    }
    send(sock, buffer, strlen(buffer), 0);
    clearBuf(buffer);
  }
  
  char buffer1[100];
  bzero(buffer1, 100);
  recv(sock, buffer1, sizeof(buffer1), 0);
  printf("%s\n", buffer1);
  close(sock);
  printf("Disconnected from the server.\n");

  return 0;

}