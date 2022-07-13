//roll number: 19CS30016

#include "rsocket.h"
#define ROLL 16 
#define MAX_SIZE 50 

 int main() 
 {
 	int M1 = r_socket(AF_INET, SOCK_MRP, 0); 
 	if (M1 < 0)
 	{
 		perror("UDP Socket Creation Failure ");
 		exit(EXIT_FAILURE); 
 	}
 	int enable = 1; 
	if(setsockopt(M1, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)	
	{
		perror("setsockopt(SO_REUSEADDR) "); 				
	} 	

 	struct sockaddr_in user1_addr; 
 	socklen_t user1_len = sizeof(user1_addr);  	
 	memset(&user1_addr, '\0', user1_len); 
 	user1_addr.sin_family = AF_INET; 
 	user1_addr.sin_addr.s_addr = INADDR_ANY; 
 	user1_addr.sin_port = htons(50000 + 2*ROLL); 

 	if(r_bind(M1,(struct sockaddr*)&user1_addr, user1_len) < 0)
 	{
 		perror("UDP Socket Bind Failed "); 
 		exit(EXIT_FAILURE); 
 	}

 	char* buffer = (char*)calloc(sizeof(char), MAX_SIZE+5); 

 	fprintf(stdout,"Enter a String of length < 50: "); 
 	fgets(buffer, MAX_SIZE, stdin);
	buffer[strlen(buffer)-1] = '\0'; 
 	
 	struct sockaddr_in user2_addr; 
 	socklen_t user2_len = sizeof(user2_addr); 
 	memset(&user2_addr, '\0', user2_len);
 	user2_addr.sin_family = AF_INET; 
 	user2_addr.sin_addr.s_addr = INADDR_ANY; 
 	user2_addr.sin_port = htons(50000 + 2*ROLL + 1);  

 	int i; 
 	for(i = 0; i < strlen(buffer);i++)
 	{
 		if(r_sendto(M1,buffer+i,sizeof(char),0,(struct sockaddr*)&user2_addr,user2_len) < 0)
 		{
 			perror("sendto() Failed "); 
 			exit(EXIT_FAILURE); 
 		}
 	}

 	r_close(M1);

 	fprintf(stderr, "Program Exit Success\n");
 	exit(EXIT_SUCCESS); 

 }