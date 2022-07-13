// File Transfer Protocol Client side
// Group members: Soumita Hait (19CS10058) and Sanyukta Deogade (19CS30016)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m" 

#define MAX_SIZE 149
#define BUFF_SIZE 100
#define COMM_SIZE 79
#define header_size 3
#define MAX_FILES 100

static int loginSuccessFlag = 0;

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 1024; i++)
        b[i] = '\0';
}

void printBuf(char* b) {
    int i;
    for (i = 0; i <= MAX_SIZE; i++) {
        if(b[i] != '\0')
            printf("%c", b[i]);
    }
}

void set ( char *str ) {
    memset(str, '\0' , MAX_SIZE+1 ) ; 
}

int stoi( char *tmp ) {
    int val ; sscanf( tmp , "%d" , &val) ; return val ;  
}

int readtimeout(int s, char buff[], int length) {
    fd_set fds;
    int n;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(s, &fds);

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if(n==0) return -2; // timeout
    if(n==-1) return -1; // error

    return read(s, buff, length);
}

int change_directory(char* Command) {
    char *Command_Name, *Argument;
    Command_Name = strtok(Command, " ");
    Argument = strtok(NULL, " ");
    if(chdir(Argument) != 0) {
        perror(RED "Directory change failed " WHITE);
        return -1;
    }
    return 0;
}

int main()
{   
    short int read_bytes1 , read_bytes2 , net_read_bytes , net_len , len,res ;
    int Error_Code; 
    int  i , header , last ;
    int recv_bytes, enable, status, process , FORK , PORT_Y ; 
    int CC_serv_len , SD_serv_len , CD_cli_len ; 
    int C1_sockfd , C2_sockfd , newC2_sockfd , File_Desc; 
    struct sockaddr_in CC_serv_addr, CD_cli_addr, SD_serv_addr ; 
    char Server_IP[MAX_SIZE+1], net_len_bit[MAX_SIZE+1] ; 
    char buffer[MAX_SIZE+1] , temp[MAX_SIZE+1],Response[MAX_SIZE+1], buffer1[MAX_SIZE+1] ;    
    char Command[MAX_SIZE+1] , FileName[MAX_SIZE+1] , Block[MAX_SIZE+1] ;
    char *PathNames[MAX_FILES+1];
    char *Command_Name , *remote_PathName, *local_PathName , *tmp,*com ;
    const char* PORT = "port" ; 
    const char* CD   = "cd"   ; 
    const char* GET  = "get"  ; 
    const char* PUT  = "put"  ; 
    const char* QUIT = "quit" ; 
    char Nul[] = "\0" ; 
    char delim[] = " ";

    // Creating socket file descriptor
    if ( (C1_sockfd = socket( AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror(RED "Control Socket Creation Failure " WHITE ) ; 
        exit(EXIT_FAILURE) ; 
    }    

    printf( BLUE "myFTP>" WHITE ) ; 
    fgets(buffer , MAX_SIZE , stdin ) ; buffer[strlen(buffer)-1] = '\0' ; 
    char *ptr = strtok(buffer, delim);
    char *ptr1,*ptr2,*ptr3;
    ptr1 = ptr;
    i = 0;
    while(ptr != NULL)
    {
        ptr = strtok(NULL, delim);
        if(i==0)
            ptr2 = ptr;
        if(i==1)
            ptr3 = ptr;
        i++;
    }

    if(strcmp(ptr1,"open"))
    {
        printf(RED "Error: open command not given");
        return 0;
    }
    if ( strlen(ptr2) == 0 ) sprintf( ptr2 , "%s" , "127.0.0.1" ) ;
    memset(&CC_serv_addr, '\0', sizeof(CC_serv_addr));
    strcpy(Command,"port ");
    strcat(Command,ptr3);
    PORT_Y = atoi(ptr3);

    CC_serv_addr.sin_family = AF_INET ;
    CC_serv_addr.sin_port = htons(PORT_Y) ; 
    CC_serv_len = sizeof(CC_serv_addr) ; 
    CC_serv_addr.sin_addr.s_addr = inet_addr(ptr2) ;

    // Connecting to the server
    if(connect( C1_sockfd , (struct sockaddr *) &CC_serv_addr , CC_serv_len ) < 0 )
    {
        perror(RED "Client unable to connect to Server " WHITE ) ;
        close(C1_sockfd) ; 
        exit(EXIT_FAILURE) ;  
    }

    else
    {
        printf(YELLOW "Connection opened successfully\n" WHITE);
        i = 0;       
        while(1)
        {
            if(i==0)
            {
                strcpy(buffer,Command);
                strcpy(buffer1,Command);
                i++;
                
            }
            else
            {
                printf(BLUE "myFTP>" WHITE );
                clearBuf(buffer);   clearBuf(Command); clearBuf(buffer1);
                set(buffer) ; fgets(buffer , MAX_SIZE , stdin) ; buffer[strlen(buffer)-1] = '\0' ; 
                set(Command); sprintf(Command , "%s" , buffer ) ; 
                set(buffer1); sprintf(buffer1 , "%s" , buffer ) ; 
            }
            
            Command_Name = strtok(buffer , " ") ;
            com = strtok(buffer, " ");
            if ( !Command_Name ) continue ; 
            FORK = 0 ; 
            if ( !strcmp(Command_Name,"get") ) 
            {
                if(loginSuccessFlag) {
                    FORK = 1 ; 
                    Command_Name = strtok(buffer1 , " ") ;
                    remote_PathName = strtok(NULL , " ") ;
                    local_PathName = strtok(NULL , " ") ;
                    if ( !local_PathName || !remote_PathName || strtok(NULL , " ") )
                    {
                        fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);                             
                    } 
                    else if ( (File_Desc = open( local_PathName , O_CREAT | O_WRONLY | O_RDONLY )) < 0 )
                    {
                        fprintf(stderr, RED "\'%s\' " WHITE, local_PathName);
                        perror("") ;                                             
                    }
                    else
                    {
                        // send command
                        if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                        {   
                            fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                            close(C1_sockfd) ; 
                            exit(EXIT_FAILURE) ;
                        }
                        // printf("Command sent\n");
                        char error_code[3] = {0};
                        if( recv ( C1_sockfd , error_code , 3 , 0 ) < 0 )
                        {
                            fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                            kill(process , SIGKILL ) ; 
                            close(C1_sockfd) ; 
                            exit(EXIT_FAILURE) ;
                        }
                        
                        Error_Code = atoi(error_code); 
                        if(Error_Code == 200) {
                            // remote file opened successfully
                            fprintf(stderr, CYAN "Remote file opened successfully\n" WHITE);

                            int i, total = 0, header, len, last, recv_bytes, count = 0;
                            short int net_len;

                            header = len = last = 0 ;
                            
                            while(1) {	
                                set(buffer) ; 
                                recv_bytes = readtimeout(C1_sockfd , buffer , MAX_SIZE);
                                if ( recv_bytes == -1 ) { 
                                    perror(RED "get Failed ") ;
                                    close(File_Desc) ;
                                    break;
                                }
                                else if (recv_bytes == -2) {
                                    if(count == 0)
                                        printf("timeout\n");
                                    break;
                                }
                                else if(recv_bytes == 0) {
                                    close(File_Desc) ;
                                    break;
                                }
                                total = total + recv_bytes;
                                for ( i = 0 ; i < recv_bytes ; i++ )
                                {   
                                    if ( len == 0 )
                                    {
                                        if ( last == 1 && header == 0 ){
                                            write(File_Desc , NULL , 1 ) ;
                                            break ; 
                                        }
                                        switch(header)
                                        {
                                            case 0 :    if ( buffer[i] == 'L' ) last = 1 ; 
                                                        break ; 
                                            case 1 :    net_len_bit[0] = buffer[i] ; 
                                                        break ; 
                                            case 2 :    net_len_bit[1] = buffer[i] ; 
                                                        memcpy( &net_len , net_len_bit , 2 ) ; 
                                                        len = ntohs(net_len) ; 
                                                        break ; 
                                            default :   
                                                        break ; 
                                        }
                                        header = ( header + 1 ) % 3 ; 
                                    }
                                    else
                                    {   
                                        write( File_Desc , buffer+i , 1 ) ; 
                                        len-- ; 
                                    }
                                }
                                count++;
                            }

                            fprintf(stderr, CYAN "Local file updated successfully\n" WHITE);
                        }
                        else if(Error_Code == 500) {
                            fprintf(stderr, RED "Error in opening remote file on server\n" WHITE);
                        }
                    }
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }
            if ( !strcmp(Command_Name,"put") ) 
            {
                if(loginSuccessFlag) {
                    FORK = 2 ; 
                    Command_Name = strtok(buffer1 , " ") ;
                    local_PathName = strtok(NULL , " ") ;
                    remote_PathName = strtok(NULL , " ") ;
                    if ( !local_PathName || !remote_PathName || strtok(NULL , " ") )
                    {
                        fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);                        
                    }     
                    else if ( (File_Desc = open( local_PathName , O_RDONLY )) < 0 )
                    {
                        fprintf(stderr, RED "\'%s\' " WHITE, local_PathName);
                        perror("") ;                                              
                    }
                    else
                    {
                        // send command
                        if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                        {   
                            fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                            close(C1_sockfd) ; 
                            exit(EXIT_FAILURE) ;
                        }
                        //printf("Command sent\n");
                        char error_code[4] = {0};
                        if( recv ( C1_sockfd , error_code , 4 , 0 ) < 0 )
                        {
                            fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                            kill(process , SIGKILL ) ; 
                            close(C1_sockfd) ; 
                            exit(EXIT_FAILURE) ;
                        }
                        
                        Error_Code = atoi(error_code); 
                        if(Error_Code == 200) {
                            // remote file opened successfully
                            fprintf(stderr, CYAN "Remote file opened successfully\n" WHITE);

                            set(buffer) ;                     
                            if ((read_bytes1 = read(File_Desc, buffer, BUFF_SIZE)) < 0)
                            {
                                perror( RED "1. Read Failure " WHITE ) ; 
                                close(File_Desc) ;
                                continue;
                            } 
                            if ( read_bytes1 == 0 )
                            {   
                                set(Block) ; 
                                sprintf( Block , "%c%c%c" , 'L' , 0 , 0 ) ; 
                                send( C1_sockfd , Block , header_size + read_bytes1 + 1 , 0 ) ; 
                                close(File_Desc) ; 
                                fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                                continue;
                            }
                            
                            while(1)
                            {   
                                set(Block);
                                net_read_bytes = htons(read_bytes1);
                                Block[0] = 'M';
                                memcpy(Block+1, &net_read_bytes, 2); 
                                memcpy(Block+3, buffer, read_bytes1);
                                
                                set(buffer) ; 
                                if ((read_bytes2 =read(File_Desc , buffer , BUFF_SIZE )) < 0) 
                                {
                                    perror(RED "2. Read Failure " WHITE);
                                    close(File_Desc); 
                                    break;
                                }                        
                                if ( read_bytes2 == 0 )
                                {                              
                                    Block[0] = 'L'; 
                                    send(C1_sockfd, Block, header_size + read_bytes1 + 1, 0);
                                    close(File_Desc); 
                                    fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                                    break;
                                }
                                send(C1_sockfd, Block, header_size + read_bytes1, 0);
                                read_bytes1 = read_bytes2;
                            }
                            // fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                        }
                        else if(Error_Code == 500) {
                            fprintf(stderr, RED "Error in opening remote file on server\n" WHITE);
                        }
                    }
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }
            if(!strcmp(Command_Name, "mput")) {
                if(loginSuccessFlag) {
                    Command_Name = strtok(buffer1 , " ") ;
                    int num_files = 0, i;
                    while( PathNames[num_files] = strtok(NULL , " ") ) {
                        num_files++;
                    }
                    for(i = 0; i < num_files; i++) {
                        if ( !PathNames[i] )
                        {
                            fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);                           
                        }     
                        else if ( (File_Desc = open( PathNames[i] , O_RDONLY )) < 0 )
                        {
                            fprintf(stderr, RED "\'%s\' " WHITE, PathNames[i]);
                            perror("") ;
                            fprintf(stderr, RED "File transfer failed\n" WHITE);
                            break;                                  
                        }
                        else
                        {
                            // send command
                            set(Command);
                            strcpy(Command, "put ");
                            strcat(Command, PathNames[i]);
                            strcat(Command, " ");
                            strcat(Command, PathNames[i]);
                            printf("Command sent: %s\n", Command);
                            if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                            {   
                                fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ;
                            }
                            // printf("Command sent\n");
                            char error_code[4] = {0};
                            int valread = readtimeout ( C1_sockfd , error_code , 4 );
                            if( valread == -1 )
                            {
                                fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                                kill(process , SIGKILL ) ; 
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ;
                            }
                            else if(valread == -2) {
                                printf("timeout\n");
                                continue;
                            }
                            
                            Error_Code = atoi(error_code); 
                            if(Error_Code == 200) {
                                // remote file opened successfully
                                fprintf(stderr, CYAN "Remote file opened successfully\n" WHITE);

                                set(buffer) ;                     
                                if ((read_bytes1 = read(File_Desc, buffer, BUFF_SIZE)) < 0)
                                {
                                    perror( RED "1. Read Failure " WHITE ) ; 
                                    close(File_Desc) ; 
                                    continue;
                                } 
                                if ( read_bytes1 == 0 )
                                {   
                                    set(Block) ; 
                                    sprintf( Block , "%c%c%c" , 'L' , 0 , 0 ) ; 
                                    send( C1_sockfd , Block , header_size + read_bytes1 + 1 , 0 ) ; 
                                    close(File_Desc) ; 
                                    fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                                    continue;
                                }
                                
                                while(1)
                                {   
                                    set(Block);
                                    net_read_bytes = htons(read_bytes1);
                                    Block[0] = 'M';
                                    memcpy(Block+1, &net_read_bytes, 2); 
                                    memcpy(Block+3, buffer, read_bytes1);
                                    
                                    set(buffer) ; 
                                    if ((read_bytes2 =read(File_Desc , buffer , BUFF_SIZE )) < 0) 
                                    {
                                        perror(RED "2. Read Failure " WHITE);
                                        close(File_Desc); 
                                        break;
                                    }                        
                                    if ( read_bytes2 == 0 )
                                    {                               
                                        Block[0] = 'L'; 
                                        send(C1_sockfd, Block, header_size + read_bytes1 + 1, 0);
                                        close(File_Desc); 
                                        fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                                        break;
                                    }
                                    send(C1_sockfd, Block, header_size + read_bytes1, 0);
                                    read_bytes1 = read_bytes2;
                                }
                                // fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                            }
                            else if(Error_Code == 500)
                            fprintf(stderr, RED "Error in opening remote file on server\n" WHITE);
                        }
                        usleep(4000000) ;
                    }
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }
            if(!strcmp(Command_Name, "mget")) {
                if(loginSuccessFlag) {
                    Command_Name = strtok(buffer1 , " ") ;
                    int num_files = 0, i;
                    while( PathNames[num_files] = strtok(NULL , " ") ) {
                        num_files++;
                    }
                    for(i = 0; i < num_files; i++) {
                        if ( !PathNames[i] )
                        {
                            fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);                               
                        }     
                        else if ( (File_Desc = open( PathNames[i] , O_CREAT | O_WRONLY | O_RDONLY )) < 0 )
                        {
                            fprintf(stderr, RED "\'%s\' " WHITE, PathNames[i]);
                            perror("") ;    
                            fprintf(stderr, RED "File transfer failed\n" WHITE); 
                            break;                                          
                        }
                        else
                        {
                            // send command
                            set(Command);
                            strcpy(Command, "get ");
                            strcat(Command, PathNames[i]);
                            strcat(Command, " ");
                            strcat(Command, PathNames[i]);
                            
                            if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                            {   
                                fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ;
                            }
                            // printf("Command sent\n");
                            char error_code[3] = {0};
                            if( recv ( C1_sockfd , error_code , 3 , 0 ) < 0 )
                            {
                                fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                                kill(process , SIGKILL ) ; 
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ;
                            }
                            
                            Error_Code = atoi(error_code);
                            if(Error_Code == 200) {
                                // remote file opened successfully
                                fprintf(stderr, CYAN "Remote file opened successfully\n" WHITE);

                                int i, total = 0, header, len, last, recv_bytes, count = 0;
                                short int net_len;

                                header = len = last = 0 ;
                                
                                while(1) {	
                                    set(buffer) ; 
                                    recv_bytes = readtimeout(C1_sockfd , buffer , MAX_SIZE);
                                    if ( recv_bytes == -1 ) { 
                                        perror(RED "get Failed ") ;
                                        close(File_Desc) ;
                                        break;
                                    }
                                    else if (recv_bytes == -2) {
                                        if(count == 0)
                                            printf("timeout\n");
                                        break;
                                    }
                                    else if(recv_bytes == 0) {
                                        close(File_Desc) ;
                                        break;
                                    }
                                    total = total + recv_bytes;
                                    for ( i = 0 ; i < recv_bytes ; i++ )
                                    {   
                                        if ( len == 0 )
                                        {
                                            if ( last == 1 && header == 0 ){
                                                write(File_Desc , NULL , 1 ) ; 
                                                break ; 
                                            }
                                            switch(header)
                                            {
                                                case 0 :    if ( buffer[i] == 'L' ) last = 1 ; 
                                                            break ; 
                                                case 1 :    net_len_bit[0] = buffer[i] ; 
                                                            break ; 
                                                case 2 :    net_len_bit[1] = buffer[i] ; 
                                                            memcpy( &net_len , net_len_bit , 2 ) ; 
                                                            len = ntohs(net_len) ; 
                                                            break ; 
                                                default :   
                                                            break ; 
                                            }
                                            header = ( header + 1 ) % 3 ; 
                                        }
                                        else
                                        {   
                                            write( File_Desc , buffer+i , 1 ) ; 
                                            len-- ; 
                                        }
                                    }
                                    count++;
                                }
                                fprintf(stderr, CYAN "Local file updated successfully\n" WHITE);
                            }
                            else if(Error_Code == 500) {
                                fprintf(stderr, RED "Error in opening remote file on server\n" WHITE);
                            }
                        }
                        usleep(100000) ;
                    }
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }
            if(!strcmp(Command_Name, "lcd")) {
                if(loginSuccessFlag) {
                    FORK = 3;
                    int val = change_directory(Command);
                    if(val == 0)
                        printf(CYAN "Directory changed successfully\n" WHITE);
                    char curr_dir[MAX_SIZE+1];
                    printf(YELLOW "Current Directory: %s\n" WHITE, getcwd(curr_dir, MAX_SIZE+1));
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }
            if(!strcmp(Command_Name, "dir")) {
                if(loginSuccessFlag) {
                    FORK = 4;
                    if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                    {   
                        fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                        close(C1_sockfd) ; 
                        exit(EXIT_FAILURE) ;
                    }
                    clearBuf(Response);
                    int count = 0, valread;
                    while(1) {
                        valread = readtimeout(C1_sockfd, Response, sizeof(Response));
                        if(valread == -1)
                        {
                            fprintf(stderr, RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ); 
                            kill(process, SIGKILL);
                            close(C1_sockfd);
                            exit(EXIT_FAILURE);
                        }
                        else if (valread == -2) {
                            if(count == 0)
                                printf("timeout\n");
                            break;
                        }
                        else {
                            printf("%s", Response);
                        }
                        count++;
                    }
                }
                else printf(RED "Authentication failed\n" WHITE);
                continue;
            }

            if(!FORK) // cd , quit , user, pass
            {
                if( send ( C1_sockfd , Command , strlen(Command)+1 , 0 ) < 0 ) 
                {   
                    fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                    close(C1_sockfd) ; 
                    exit(EXIT_FAILURE) ;
                }
                clearBuf(Response);
                if( recv ( C1_sockfd ,Response , MAX_SIZE+1 , 0 ) < 0 )
                {
                    fprintf( stderr , RED "Server Closed Connection - " WHITE "Closing Connections and Quitting....\n" ) ; 
                    kill(process , SIGKILL ) ; 
                    close(C1_sockfd) ; 
                    exit(EXIT_FAILURE) ;
                }
                
                Error_Code = atoi(Response);
                // printf("Error code is %d\n",Error_Code);
                switch(Error_Code) 
                {               
                    case 200 :  if ( !strcmp(Command_Name , "port" ) ){                           
                                    printf(GREEN "Connected to the server successfully\n" WHITE);                              
                                    break ; 
                                }
                                if ( !strcmp(Command_Name , "cd" ) ){
                                    printf(CYAN "Directory Changed Successfully\n" WHITE);                         
                                    break ; 
                                }
                                if ( !strcmp(Command_Name , "user" ) ){
                                    printf(YELLOW "Enter Password command\n" WHITE);                               
                                    break ; 
                                }
                                if ( !strcmp(Command_Name , "pass" ) ){
                                    printf(GREEN "Login Successful\n" WHITE);
                                    loginSuccessFlag = 1;                              
                                    break ; 
                                }
                                    
                    case 421 :  printf(CYAN "Closing Connections and Quitting....\n" WHITE);
                                close(C1_sockfd) ; 
                                exit(EXIT_SUCCESS) ;    

                    case 501 :  printf(RED "Invalid Arguments, Please Try Again\n" WHITE); 
                                i--; 
                                break ; 

                    case 600 :  printf(RED "Wrong Command given\n" WHITE);
                                break;

                    case 550 :  printf(RED "FATAL ERROR - Incorrect Port Number!\n" WHITE);
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ; 

                    case 500 :  if ( !strcmp(Command_Name , "user" ) ){
                                        printf(RED "Wrong Username\n" WHITE);
                                        break;  
                                }
                                if ( !strcmp(Command_Name , "pass" ) ){
                                        printf(RED "Wrong Password, Enter Username,password again\n" WHITE);
                                        loginSuccessFlag = 0;
                                        break;  
                                }
                                if(!strcmp(Command_Name, "cd")) {
                                    printf(RED "Directory change failed\n" WHITE);
                                    break;
                                }


                    default :   printf(RED "Some Error Occured on Server Side - Closing Connections and Quitting....\n" WHITE);
                                close(C1_sockfd) ; 
                                exit(EXIT_FAILURE) ;
                }
                    
                continue ; 
            
            }
            
        }
    
    }
}