// Soumita Hait (19CS10058) and Sanyukta Deogade (19CS30016)
#include "rsocket.h"

// Global variables: shared between the threads
circ_buffer recv_table; // Received-message Table
UnACK unack_table; // Unacknowledged-message Table

short int curr_SEQ_NO = -1; // Global UNIQUE_SEQ_NO Counter
int NumRecv = 0; // Global Counter for Number of Packets Received
int NumPacket = 0; // Global Counter for Number of Packets 
char DATA_MSGTYPE = 'D'; // Message Type distinguisher
char ACK_MSGTYPE = 'A';	// Message Type distinguisher

// Thread ids
pthread_t r_tid;
pthread_t s_tid;

// Signal handler
void sigHandler(int sig) {
	if(sig == SIGUSR1) {
		pthread_exit(0);
	}
}

// Getting the current time
void GetCurrentTime(struct timeval *time_struct) {
	gettimeofday(time_struct, NULL);
}

/*** Creating an MRP Socket and dynamically allocate memory to the received-message table 
and unacknowledged-message table and create threads R and S ***/
int r_socket(int domain, int type, int protocol) {	
	// Creation of MRP type Socket 
    int socketfd;
	if(type != SOCK_MRP) {
		printf("MRP Socket Creation Failed : Invalid Socket Type\n");
		return -1; 
	}

	if((socketfd = socket(domain, SOCK_DGRAM, protocol)) < 0) {
		perror("MRP Socket Creation Failed");
		return -1; 
	}

    // Received Message Table
	recv_table.buffer = (MsgData*)calloc(sizeof(MsgData), MAXSIZE_TABLE);
	if(recv_table.buffer == NULL) {
		printf("Memory allocation of recv_table buffer failed\n");
		return -1;
	}
	for(int i = 0; i < MAXSIZE_TABLE; i++) {
		recv_table.buffer[i].msg = (char*)malloc(sizeof(char));
	}
	recv_table.start = 0;
	recv_table.end = 0;
	recv_table.size = 0;
	recv_table.max_size = MAXSIZE_TABLE;
	if(pthread_mutex_init(&(recv_table.Lock), NULL) != 0) {
		perror("Mutex lock creation of recv_table failed");
		return -1;
	}
	if(pthread_cond_init(&(recv_table.Empty), NULL) != 0) {
		perror("Condition variable Empty of recv_table failed");
		return -1;
	}
	if(pthread_cond_init(&(recv_table.Full), NULL) != 0) {
		perror("Condition variable Full of recv_table failed");
		return -1;
	}
	
	// Unacknowledged Message Table indexed by Message sequence numbers
	unack_table.Table = (AckData**)malloc(sizeof(AckData*) * MAXSIZE_TABLE);
	if(unack_table.Table == NULL) {
		printf("Memory allocation of unack_table Table failed\n");
		return -1; 
	}
	unack_table.size = 0;
	if(pthread_mutex_init(&(unack_table.Lock), NULL) != 0) {
		perror("Mutex lock creation of unack_table failed");
		return -1;
	}
	if(pthread_cond_init(&(unack_table.NonEmpty), NULL) != 0) {
		perror("Condition variable NonEmpty of unack_table failed");
		return -1;
	}

	// Initializing the unacknowledged-message table
	for(int i = 0; i < MAXSIZE_TABLE; i++) 
		unack_table.Table[i] = NULL;
	
	// Creation of Thread R   
    int* socketfdp =(int*)malloc(sizeof(int));
	*socketfdp = socketfd; 
	if(pthread_create(&r_tid, NULL, Thread_R, socketfdp) < 0) {
		perror("Thread R Creation Failed "); 
		return -1; 
	}

    // Creation of Thread S   
	if(pthread_create(&s_tid, NULL, Thread_S, socketfdp) < 0) {
		perror("Thread S Creation Failed "); 
		return -1; 
	}

	printf("MRP Socket created, Memory allocated, Threads R and S created\n");
	
	return socketfd; 
}

int r_bind(int socketfd, const struct sockaddr *addr, socklen_t addr_len) {	
	if(bind(socketfd, addr, addr_len) < 0) {
		perror("MRP Socket Binding Failed ");
		return -1; 
	}
	return 0;
}

int r_sendto(int socketfd, const void* Msg, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_addr_len) {	
	// Generate Valid Unique Message Seq No 	
	curr_SEQ_NO++; // Incrementing from previous value (Unique)

    // Already occupied curr_SEQ_NO in UnACK Table(Invalid)
	if(unack_table.Table[curr_SEQ_NO] != NULL) {
		printf("Message sequence number has already been used\n");
		return -1;
	}

	// Generate Data Message Packet 
	int new_seq_no = curr_SEQ_NO;
	int packlen = 0;
	char* Packet =(char*)calloc(sizeof(char), MAXSIZE_PACKET);
	memcpy(Packet + packlen, &DATA_MSGTYPE, TYPE_SIZE);
	packlen += TYPE_SIZE; // Type
	memcpy(Packet + packlen, &new_seq_no, SEQ_NO_SIZE);
	packlen += SEQ_NO_SIZE; // Sequence no
	memcpy(Packet + packlen,(char*)Msg, length);
	packlen += length; // Message
	
	// Sending the created Packet  	
	int sent_packlen = -1; 
	if((sent_packlen = sendto(socketfd, Packet, packlen, flags, dest_addr, dest_addr_len)) != packlen) {
		perror("MRP Socket r_sendto() Failed");
		return -1;
	}
	
	NumPacket++; 
	printf(YELLOW "Packet Count : %2d\t" BLUE "Packet Sent: ", NumPacket);
	printf(BLUE "%c%d%s\n" WHITE, Packet[0], curr_SEQ_NO, Packet+3);

	// Successfully Sent, now storing in UnACK Table
	// Creating AckData for the Sent Message 
	struct timeval curr_time; 
	GetCurrentTime(&curr_time); // Setting the current time for MsgData
	AckData new_msg = {Packet, packlen, flags, dest_addr, dest_addr_len, curr_time}; 	 

	// Adding message to unacknowledged-messages Table
	pthread_mutex_lock(&(unack_table.Lock));
	unack_table.Table[curr_SEQ_NO] =(AckData*)malloc(sizeof(AckData)); // Allocating Memory to new unack_table.Table Entry 		
	*unack_table.Table[curr_SEQ_NO] = new_msg; // unack_table.Table[id] points to value of new_msg  
	unack_table.size++; // Increment Count of Outstanding Messages
	pthread_mutex_unlock(&(unack_table.Lock));

	return length;
}

void GetMessage(MsgData* msg_Info, char* message) {
	for(int i=0; i < (msg_Info->msg_len); i++) {
		message[i] = msg_Info->msg[i];
	}
}

int r_recvfrom(int socketfd, void* message, size_t length, int flags, struct sockaddr *source_addr, socklen_t *source_addr_len) {	
	pthread_mutex_lock(&(recv_table.Lock));

	// Blocking till Table is Empty
	while(!recv_table.size) {	
		pthread_cond_wait(&recv_table.Empty, &recv_table.Lock);
	}

	// Get the First element from the FIFO Circular queue
	MsgData* msg_data = (MsgData*)malloc(sizeof(MsgData));
	*msg_data = recv_table.buffer[recv_table.start];
	recv_table.start = (recv_table.start+1) % recv_table.max_size;
	recv_table.size--; // Decrement Table Count
	pthread_cond_signal(&recv_table.Full); // Signal: Table may not be Full anymore

	// Assigning Values to the Value-Result Arguments
	GetMessage(msg_data, (char*)message);
	*source_addr = *(msg_data->source_addr);
	*source_addr_len = msg_data->source_addr_len;

	pthread_mutex_unlock(&(recv_table.Lock));

	return(msg_data->msg_len); // Returns the Length of the Message Received  
	
}

int r_close(int socketfd) {		
	signal(SIGUSR1, sigHandler);
	printf(RED "Waiting for ACKs...\n" WHITE);

	pthread_mutex_lock(&unack_table.Lock);

    // Blocking until all acknowledgements are received
    while(unack_table.size){					
        pthread_cond_wait(&unack_table.NonEmpty, &unack_table.Lock);
    }
    printf(CYAN "All ACKs Received\n" WHITE);

    free(recv_table.buffer);
	free(unack_table.Table);
	pthread_kill(r_tid, SIGUSR1);
	pthread_join(r_tid, NULL);
    pthread_kill(s_tid, SIGUSR1);
	pthread_join(s_tid, NULL);
    close(socketfd);
		
	pthread_mutex_unlock(&unack_table.Lock);

	pthread_mutex_destroy(&recv_table.Lock);
	pthread_cond_destroy(&recv_table.Empty);
	pthread_cond_destroy(&recv_table.Full);
	pthread_mutex_destroy(&unack_table.Lock);
	pthread_cond_destroy(&unack_table.NonEmpty);

	printf("Threads terminated safely and MRP Socket closed\n");
    
	return 0; 
}

int dropMessage(float p) {	
    struct timeval tv; 
	GetCurrentTime(&tv);
	srand(tv.tv_usec);
	float val = (float)rand()/RAND_MAX;
    if (val < p)
        return 1;
    return 0;
}

void* Thread_R(void* param) {
	int socketfd = *((int*)param);

	char* pkt; 
	size_t recv_packlen = MAXSIZE_PACKET; 
	struct sockaddr source;
	socklen_t source_addr_len = sizeof(source);
	int valread;
	pkt = (char*)malloc(MAXSIZE_PACKET * sizeof(char));

	while(1) {
		// Waiting for a packet to come in a recvfrom() call
		if((recv_packlen = recvfrom(socketfd, pkt, recv_packlen, 0, &source, &source_addr_len)) < 0) {
			perror("Receive Error");
			exit(EXIT_FAILURE);
		}

		// Packet contents: type|sequence_number|message	
		char type = *pkt; // Extracting the type of packet
		pkt = pkt + TYPE_SIZE;

		// Handling Data Messages
		if(type == DATA_MSGTYPE) {	
			NumRecv++; // Incrementing the number of data messages received

			if(dropMessage(P)) {
				// Dropping the data message
				continue;
			}
			
			// Extracting the sequence number of the Data message
			int seq_no = *pkt;
			
			// printf(CYAN "##%s" WHITE, pkt+SEQ_NO_SIZE);
				
			// Creation of a Table Node
			MsgData msg_Info = {(pkt+SEQ_NO_SIZE), (recv_packlen-TYPE_SIZE-SEQ_NO_SIZE), &source, source_addr_len};

			pthread_mutex_lock(&(recv_table.Lock));

			// Blocking till Table is Full 
			while(recv_table.size == recv_table.max_size) {
				pthread_cond_wait(&recv_table.Full, &recv_table.Lock);
			}

			// Enqueuing New Message into End of Table
			recv_table.buffer[recv_table.end].msg_len = recv_packlen-TYPE_SIZE-SEQ_NO_SIZE;
			recv_table.buffer[recv_table.end].msg = (char*)malloc(sizeof(char) * (recv_packlen-TYPE_SIZE-SEQ_NO_SIZE));
			memcpy(recv_table.buffer[recv_table.end].msg, pkt+SEQ_NO_SIZE, recv_packlen-TYPE_SIZE-SEQ_NO_SIZE);
			recv_table.buffer[recv_table.end].source_addr = &source;
			recv_table.buffer[recv_table.end].source_addr_len = source_addr_len;
			recv_table.end = (recv_table.end+1)%recv_table.max_size;
			recv_table.size++;			
			pthread_cond_signal(&recv_table.Empty); // Signaling Table is no Longer Empty

			pthread_mutex_unlock(&(recv_table.Lock));
			
			// Creating and Sending out the ACK 	

			// Creating the ACK_Packet :: |TYPE|SEQ_NO|
			int ACKPacket_Len = 0; 
			char* ACK_Packet = (char*)calloc(sizeof(char),MAXSIZE_PACKET); 	
			memcpy(ACK_Packet + ACKPacket_Len, &ACK_MSGTYPE, TYPE_SIZE); 
			ACKPacket_Len += TYPE_SIZE; // Type
			memcpy(ACK_Packet + ACKPacket_Len, &seq_no, SEQ_NO_SIZE); 
			ACKPacket_Len += SEQ_NO_SIZE; // SEQ_NO

			int sent_Packet_Len = -1;  
			if((sent_Packet_Len = sendto(socketfd, ACK_Packet, ACKPacket_Len, 0, &source, source_addr_len)) != ACKPacket_Len){
				perror("MRP Socket ACK Sending Failed "); 
				exit(EXIT_FAILURE); 
			}
		}

		// Handling Acknowledegement Messages
		if(type == ACK_MSGTYPE) {	
			if(dropMessage(P)) {
				printf(RED "Dropping Acknowledgement\n" WHITE);
				continue;
			}
				
			// Extracting the sequence number of the Acknowledgement message
			int seq_no = *pkt;			
			printf(CYAN "ACK %d%s received.\n" WHITE, seq_no, pkt + SEQ_NO_SIZE);
			
			// Checking if the sequence number is valid
			if(seq_no < 0 || MAXSIZE_TABLE <= seq_no) {
				printf(RED "Invalid Message Sequence Number: Out of Bounds\n" WHITE );
				exit(EXIT_FAILURE);
			}

			pthread_mutex_lock(&(unack_table.Lock));

			// Packet acknowledged and removed from Unacknowledged-messages table 
			if(unack_table.Table[seq_no] != NULL) {
				free(unack_table.Table[seq_no]);
				unack_table.Table[seq_no] = NULL;
				unack_table.size--;
				pthread_cond_signal(&unack_table.NonEmpty); // Signaling Outstanding Messages maybe 0 
			}

			pthread_mutex_unlock(&(unack_table.Lock));
		}
	}

	exit(EXIT_SUCCESS);
}

void* Thread_S(void* param) {
	int socketfd = *((int*)param);
	int timeouts = 0, count = 0, valread;

	while(timeouts < MAX_WAITING) {
		sleep(T);
		struct timeval curr_time; 
		GetCurrentTime(&curr_time);
		
		count = 0;

		// Checking for unacknowledged messages in the unacknowledged messages table
		for (int i = 0; i < MAXSIZE_TABLE; i++) {
			if (unack_table.Table[i] != NULL) {
				pthread_mutex_lock(&(unack_table.Lock)); 
				// checking for time-out
				if((curr_time.tv_sec - (unack_table.Table[i]->msg_time).tv_sec) > 2*T) {
					// Resending packet				
					if ((valread = sendto(socketfd, unack_table.Table[i]->Packet, unack_table.Table[i]->packet_len, 
													unack_table.Table[i]->flags, unack_table.Table[i]->dest_addr, 
													unack_table.Table[i]->dest_addr_len)) != unack_table.Table[i]->packet_len)
					{
						perror("MRP Socket Retransmission Failed"); 
						exit(EXIT_FAILURE);
					}
					// Updating Sending time in the table
					GetCurrentTime(&(unack_table.Table[i]->msg_time)); 
					printf(GREEN "Retransmitted Packet %d\n" WHITE, i);
					count++; 
				}
				pthread_mutex_unlock(&(unack_table.Lock));
			}	
		}
		if(count == 0)
			timeouts++; 
		else 
			timeouts = 0; 
	}

	exit(EXIT_SUCCESS); 
}