#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_INPUT_SIZE 100

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 100; i++)
        b[i] = '\0';
}


int main(){

  char *ip = "127.0.0.1";
  int port = 5580;
  int server_sock, client_sock;
  int characters = 0; int words = 0; int sentences = 0;
  char charnum[1000]; char wordnum[1000]; char sentnum[1000];
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[100];
  int n;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");

  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n", port);

  listen(server_sock, 5);
  printf("Listening...\n");

  while(1){
    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    printf("[+]Client connected.\n");
    clearBuf(buffer);
    while(1)
    {
      recv(client_sock, buffer, sizeof(buffer), 0);
      //printf("Received %s %ld\n",buffer,strlen(buffer));
      if(strlen(buffer) ==0 )
      {
           char res[100];
        
        sprintf(charnum, "%d", characters);
        //str1 = string.valueOf(charnum);
        //charnum[strlen(charnum)] = " ";
        sprintf(wordnum, "%d", words);
         //str2 = string.valueOf(wordnum);
        //wordnum[strlen(wordnum)] = " ";
        sprintf(sentnum, "%d", sentences);
         //str3 = string.valueOf(sentnum);
         //char result[100] = charnum+wordnum+sentnum; 
        //itoa(characters,charnum,10); itoa(words,wordnum,10); itoa(sentences,sentnum,10);
        strcpy(res,"characters:");
        strcat(res,charnum);
        strcat(res,"\n");
        strcat(res,"words: ");
        strcat(res,wordnum);
        strcat(res,"\n");
        strcat(res, "sentences:"); 
        strcat(res,sentnum);
        int j = 0; 
        char buffer1[100]; 
        clearBuf(buffer1);
        strcpy(buffer1,res);
        //printf("results is :%s\n",buffer1);
        //printf("sending results");
        send(client_sock, buffer1, strlen(buffer1), 0);
        break;
      }
       for(int i=0;i<strlen(buffer);i++)
      {
          
          if(((int)buffer[i]!= 32) && ((int)buffer[i] != 46))
          {
            characters++;
          }
          if((int)buffer[i] == 32 && (int)buffer[i-1] != 32)
          {
              words++;
          }

          if((int)buffer[i] == 46)
          {
              sentences++;
              words++;
          } 
          //printf("char: %c %d,words: %d,chars: %d\n",buffer[i],buffer[i],words,characters);
      }
      words = words-1;
      clearBuf(buffer);
    }
    
  
     
    //    bzero(buffer,100); 
    
    // //char charnum[1000]; char wordnum[1000]; char sentnum[1000];
    // printf("characters: %d\n", characters);
    // printf("words %d\n", words);
    // printf("sentences %d\n", sentences);
    // char res[100];
    
    // sprintf(charnum, "%d", characters);
    // //str1 = string.valueOf(charnum);
    // //charnum[strlen(charnum)] = " ";
    // sprintf(wordnum, "%d", words);
    //  //str2 = string.valueOf(wordnum);
    // //wordnum[strlen(wordnum)] = " ";
    // sprintf(sentnum, "%d", sentences);
    //  //str3 = string.valueOf(sentnum);
    //  //char result[100] = charnum+wordnum+sentnum; 
    // //itoa(characters,charnum,10); itoa(words,wordnum,10); itoa(sentences,sentnum,10);
    // strcpy(res,"characters:");
    // strcat(res,charnum);
    // strcat(res,"\n");
    // strcat(res,"words: ");
    // strcat(res,wordnum);
    // strcat(res,"\n");
    // strcat(res, "sentences:"); 
    // strcat(res,sentnum);
    // int j = 0; 
    // char buffer1[100]; 
    // bzero(buffer1,100);
    // strcpy(buffer1,res);
    // printf("%s\n",buffer1);
    // send(client_sock, buffer1, strlen(buffer1), 0);

    close(client_sock);
    printf("[+]Client disconnected.\n\n");

  }

  return 0;
}