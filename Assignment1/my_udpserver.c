#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 100; i++)
        b[i] = '\0';
}

void main(int argc, char **argv)
{
  if(argc != 2){
    printf("Usage: %s <port>\n", argv[0]);
    exit(0);
  }
  int words = 0; int characters = 0; int sentences = 0;
  int port = atoi(argv[1]);
  int sockfd;
  struct sockaddr_in si_me, si_other;
  char buffer[100];//buffer1[100];
  char charnum[1000]; char wordnum[1000]; char sentnum[1000];
  socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM,0);

  memset(&si_me, '\0', sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = inet_addr("127.0.0.1");

  bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
  addr_size = sizeof(si_me);

  //printf("Debugging\n");
  while(1)
  {
    recvfrom(sockfd, buffer, 100, 0, (struct sockaddr*)& si_other, &addr_size);
   // printf("%ld\n",strlen(buffer));
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
      sendto(sockfd, buffer1, 100, 0, (struct sockaddr*)&si_other, sizeof(si_other));
      return;
    }
    //printf("buffer: %s\n",buffer);
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
    //    printf("chara: %d\n", characters);
    // printf("word %d\n", words);
    // printf("sent %d\n", sentences);
    
  }
 

  //printf("[+]Data Received");
    //return 0;
}