#ifndef _rsocket_h_
#define _rsocket_h_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#define T 2
#define P 0.2
#define MAX_WAITING 120/T
#define SOCK_MRP -50
#define MAXSIZE_MSG 100
#define MAXSIZE_TABLE 50
#define TYPE_SIZE sizeof(char)
#define SEQ_NO_SIZE sizeof(short int)
#define MAXSIZE_PACKET (MAXSIZE_MSG + TYPE_SIZE + SEQ_NO_SIZE)

#define BLACK "\033[1;30m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE  "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m"

// Structure for every Node of the Receive Table Circular Queue
typedef struct _MsgData {
	char* msg;
	int msg_len;
	struct sockaddr* source_addr;
	socklen_t source_addr_len;  	
} MsgData;

// Wrapper around the different components of the Receive Table
typedef struct _circ_buffer {
	MsgData* buffer; 
	int start; 
	int end;
	int size;
	int max_size; 
	pthread_mutex_t Lock;
	pthread_cond_t Empty;  
	pthread_cond_t Full;  	 
} circ_buffer;

// Structure for every Entry of the UnACK Message Table
typedef struct _AckData {
	char* Packet;
	int packet_len;
	int flags; 
	const struct sockaddr* dest_addr;
	socklen_t dest_addr_len;  
	struct timeval msg_time; 
} AckData; 

// Wrapper around the different components of the UnACK Table
typedef struct _UnACK {
	AckData** Table; 
	int size; 
	pthread_mutex_t Lock;
	pthread_cond_t NonEmpty;  	 
} UnACK; 

int r_socket (int domain, int type, int protocol);
int r_bind (int socket_fd, const struct sockaddr *address, socklen_t address_len);
int r_sendto (int socket_fd, const void* message, size_t length, int flags, const struct sockaddr *dest_address, socklen_t dest_len);
int r_recvfrom (int socket_fd, void* message, size_t length, int flags, struct sockaddr *source_address, socklen_t *source_len);
int r_close (int socket_fd);

int dropMessage (float p); 
void* Thread_R (void*); 
void* Thread_S (void*); 
void sigHandler (int); 

#endif 