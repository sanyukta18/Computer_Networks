//roll number: 19CS30016

#include "rsocket.h"
#define ROLL 16 
#define MAX_SIZE 50

 int main() 
 {
 	int M2 = r_socket(AF_INET, SOCK_MRP, 0); 
 	if(M2 < 0)
 	{
 		perror("UDP Socket Creation Failure"); 
 		exit(EXIT_FAILURE); 
 	} 
 	int enable = 1; 
	if(setsockopt(M2, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)	
	{
		perror("setsockopt(SO_REUSEADDR)"); 				
	}	
 	
 	struct sockaddr_in user2_addr; 
 	socklen_t user2_len = sizeof(user2_addr); 
 	memset(&user2_addr, '\0', user2_len);
 	user2_addr.sin_family = AF_INET; 
 	user2_addr.sin_addr.s_addr = INADDR_ANY; 
 	user2_addr.sin_port = htons(50000 + 2*ROLL + 1);  

 	if(r_bind(M2,(struct sockaddr*)&user2_addr, user2_len) < 0)
 	{
 		perror("UDP Socket Bind Failed "); 
 		exit(EXIT_FAILURE); 
 	}	

	printf("Socket Bound to Port and Waiting for a Receive...\n");

 	while(1)
 	{	
 		char c[MAX_SIZE] = {0};  	
 		int recv_bytes = 0; 
 		struct sockaddr_in user1_addr; 
 		socklen_t user1_len = sizeof(user1_addr);

 		if((recv_bytes = r_recvfrom(M2,c,MAX_SIZE,0,(struct sockaddr*)&user1_addr,&user1_len)) < 0)
 		{
 			perror("recvfrom() Failed "); 
 			exit(EXIT_FAILURE); 
 		}

		if(strlen(c) > 0) {
		for(int i = 0; i < MAX_SIZE; i++)
			if(c[i] != '\0')
				printf(CYAN "%c\n" WHITE, c[i]);
		}
 	}

	return 0;

}