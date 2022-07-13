// File Transfer Protocol Server side
// Group members: Soumita Hait (19CS10058) and Sanyukta Deogade (19CS30016)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // defines the hostent structure 

/** Path to file user.txt containing user details **/
// CHANGE THIS!
#define USER_PATH "/home/soumitahait/networks/server/user.txt"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m" 
#define RESET "\033[0m" 

#define PORT_X 50000   // PORT NO of server, fixed for this server
#define MAX_SIZE 149
#define BUFF_SIZE 100
#define COMM_SIZE 79
#define header_size 3

unsigned short PORT_Y ; 
const char* PORT = "port" ; 
const char* CD   = "cd"   ; 
const char* GET  = "get"  ; 
const char* PUT  = "put"  ; 
const char* QUIT = "quit" ;
char Nul[] = "\0"; 
char *passwrd;
static int loginSuccessFlag = 0;

void clearBuf(char* b)
{
    int i;
    for (i = 0; i < 1024; i++)
        b[i] = '\0';
}

void set(char *str) 
{
    memset(str, '\0' , MAX_SIZE+1 ) ; 
}

int parse( char *Command_Name ){
    if ( !strcmp( Command_Name , "port" ) ) return 1 ; 
    else if ( !strcmp( Command_Name , CD   ) ) return 2 ; 
    else if ( !strcmp( Command_Name , GET  ) ) return 3 ; 
    else if ( !strcmp( Command_Name , PUT  ) ) return 4 ;
    else if ( !strcmp( Command_Name ,"quit" ) ) return 5 ;
    else if ( !strcmp( Command_Name ,"user") ) return 6 ;  
    else if ( !strcmp( Command_Name ,"pass") ) return 7 ; 
    else if ( !strcmp( Command_Name ,"dir") ) return 8 ;                               
    else return 0 ; 
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

short int parse_first ( char *Command )
{   
    int code , Port_No  ; 
    char *Command_Name , *Argument ; 
    Command_Name = strtok(Command , " ") ; 
    printf("Command name is %s\n",Command_Name);
    code = parse(Command_Name) ; 
    if ( code == 0 )                                            // Unrecognisable Command 
                    return (short int)502 ;   
    if ( code == 5 )                                            // Quit Command 
                    return (short int)421 ;       
    if ( code != 1 )                                            // Non 'port arg' command 
                    return (short int)600 ; 
    
    Argument = strtok(NULL, " ") ; 
    Port_No = atoi(Argument);
    if ( !(20000 <= Port_No && Port_No < 65535) )                // Invalid Port Number 
                    return (short int)550 ; 
    
    PORT_Y = Port_No ;     
    printf("PORTY is %d\n",PORT_Y);                     // Store Valid Port Number in Global Variable 
                    return (short int)200 ;            // Return 'ALL GOOD'     
}

short int parse_first2 ( char *Command )
{   
    int code ; 
    char *Command_Name , *Argument, *usr ; 
    Command_Name = strtok(Command , " ") ; 
    printf("Command name is %s\n",Command_Name);
    code = parse(Command_Name) ; 
    if ( code != 6 )                                            // Non 'port arg' command 
                    return (short int)600 ; 
    
    Argument = strtok(NULL, " ") ;
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(USER_PATH, "r");

    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) 
    {
         usr = strtok(line," ");
         passwrd = strtok(NULL," ");
         passwrd = strtok(passwrd,"\n");
         if(!strcmp(usr,Argument))
         {
             return (short int)200 ;
         }
       
    }
   
    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);     
     return (short int)500;   
}
short int password_check(char *Command)
{

    int code ; 
    char *Command_Name , *Argument, *usr ; 
    //char Nul ='\0'; 
    Command_Name = strtok(Command , " ") ; 
    printf("Command name is %s\n",Command_Name);
    code = parse(Command_Name) ; 
    if ( code != 7 )                                            // Non 'port arg' command 
        return (short int)600 ; 
    
    Argument = strtok(NULL, " ") ; 
    printf("Password stored is %s\n",passwrd);
    printf("Password from command is %s\n",Argument);
    if(!strcmp(passwrd,Argument))
    {
        return (short int)200 ;
    }    
     return (short int)500;   
}

short int change_directory(char* Command) {
    char *Command_Name, *Argument;
    Command_Name = strtok(Command, " ");
    printf("Command name: %s\n", Command_Name);
    Argument = strtok(NULL, " ");
    printf("Directory: %s\n", Argument);
    if(chdir(Argument) != 0) {
        perror("Directory change failed ");
        return (short int)500;
    }
    return (short int)200;
}

int directory_listing(int newS1_sockfd) {
    DIR *dp;
    struct dirent *ep;
    char curr_dir[MAX_SIZE+1] = {0};
    dp = opendir(getcwd(curr_dir, MAX_SIZE+1));
    if(dp != NULL) {
        while(ep = readdir(dp)) {
            int len = strlen(ep->d_name);
            char name[len+2];
            strcpy(name, ep->d_name);
            strcat(name, "\n");
            printf("%s", name);
            if(send(newS1_sockfd, name, strlen(name), 0) == -1)
                perror("send");
        }
        (void) closedir (dp);
    }
    if(send(newS1_sockfd, "END-OF-LIST\n", sizeof("END-OF-LIST\n"), 0) == -1)
        perror("send");
    return 0;
}

int put(char* Command, int newS1_sockfd) {
    char *Command_Name, *local_PathName, *remote_PathName, *tmp;
    char FileName[MAX_SIZE+1], buffer[MAX_SIZE+1], net_len_bit[MAX_SIZE+1];
    Command_Name = strtok(Command, " ");
    printf("Command name: %s\n", Command_Name);
    local_PathName = strtok(NULL, " ");
    printf("Local Pathname: %s\n", local_PathName);
    remote_PathName = strtok(NULL, " ");
    printf("Remote Pathname: %s\n", remote_PathName);
    char error_code[4] = {0};
    int i, total = 0, header, len, last, recv_bytes, File_Desc;
    short int net_len;

    header = len = last = 0 ;

    if ( !local_PathName || !remote_PathName || strtok(NULL , " ") )
    {
        fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);
        return -1;
    }     
    else if ( (File_Desc = open( remote_PathName , O_CREAT | O_WRONLY | O_RDONLY )) < 0 )
    {
        fprintf(stderr, RED "\'%s\' " WHITE, remote_PathName);
        perror("open") ;
        send(newS1_sockfd,"500",strlen("500"),0);
        return -1;
    }
    else {
        //successfully opened file for writing
        send(newS1_sockfd,"200",strlen("200"),0);

        while(1) {	
            set(buffer) ; 
            recv_bytes = readtimeout(newS1_sockfd , buffer , MAX_SIZE );
            if ( recv_bytes == -1 ) { 
                perror(RED "get Failed ") ;
                close(File_Desc) ;
                return -1;
            }
            else if (recv_bytes == -2) {
                printf("1$timeout\n");
                return 0;
            }
            total = total + recv_bytes;
            for ( i = 0 ; i < recv_bytes ; i++ ) {   
                if ( len == 0 ) {
                    if ( last == 1 && header == 0 ) {
                        write(File_Desc , NULL , 1 ) ;
                        break ; 
                    }
                    switch(header) {
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
                else {   
                    write( File_Desc , buffer+i , 1 ) ; 
                    len-- ; 
                }
            }
        }
        return 0;
    }
    return 0;
}

int get(char* Command, int newS1_sockfd) {
    char *Command_Name, *local_PathName, *remote_PathName, *tmp;
    char FileName[MAX_SIZE+1], buffer[MAX_SIZE+1], net_len_bit[MAX_SIZE+1], Block[MAX_SIZE+1];
    Command_Name = strtok(Command, " ");
    printf("Command name: %s\n", Command_Name);
    remote_PathName = strtok(NULL, " ");
    printf("Remote Pathname: %s\n", remote_PathName);
    local_PathName = strtok(NULL, " ");
    printf("Local Pathname: %s\n", local_PathName);
    char error_code[4] = {0};
    short int read_bytes1 , read_bytes2, net_read_bytes;
    int File_Desc;

    if ( !local_PathName || !remote_PathName || strtok(NULL , " ") )
    {
        fprintf(stderr, RED "Invalid Arguments\nWaiting for next command.\n" WHITE);
        return -1;
    }     
    else if ( (File_Desc = open( remote_PathName , O_RDONLY )) < 0 )
    {
        fprintf(stderr, RED "\'%s\' " WHITE, remote_PathName);
        perror("open") ;
        send(newS1_sockfd,"500",strlen("500"),0);
        return -1;
    }
    else {
        //successfully opened file for reading
        send(newS1_sockfd,"200",strlen("200"),0);

        while(1) {	
            set(buffer) ;                     
            if ((read_bytes1 = read(File_Desc, buffer, BUFF_SIZE)) < 0)
            {
                perror( RED "1. Read Failure " WHITE ) ; 
                close(File_Desc) ;
                return -1;
            } 
            if ( read_bytes1 == 0 )
            {   
                set(Block) ; 
                sprintf( Block , "%c%c%c" , 'L' , 0 , 0 ) ; 
                send( newS1_sockfd , Block , header_size + read_bytes1 + 1 , 0 ) ; 
                close(File_Desc) ;
                fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                return 0;
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
                    return -1;
                }                        
                if ( read_bytes2 == 0 )
                {                             
                    Block[0] = 'L'; 
                    send(newS1_sockfd, Block, header_size + read_bytes1 + 1, 0);
                    close(File_Desc); 
                    fprintf(stderr, CYAN "Remote file updated successfully\n" WHITE);
                    return 0;
                }
                send(newS1_sockfd, Block, header_size + read_bytes1, 0);
                read_bytes1 = read_bytes2;
            }
        }
        return 0;
    }
}

int main () 
{   
    short int read_bytes1 , read_bytes2 , net_read_bytes , net_len , len ; 
    short int Error_Code, Response ; 
    int i , header, last, recv_bytes, total, code, stop, next, process, status, enable , val;     
    int S1_sockfd, newS1_sockfd, S2_sockfd, File_Desc ; 
    int SC_serv_len, CC_cli_len, CD_cli_len;
    char net_len_bit[MAX_SIZE+1] ; 
    char buffer[MAX_SIZE+1] , buffer1[MAX_SIZE+1] ; 
    char Command[MAX_SIZE+1] , FileName[MAX_SIZE+1] ; 
    char Header[MAX_SIZE+1] , Block[MAX_SIZE+1] ; 
    char *Command_Name, *remote_PathName,*local_PathName,*DirName , *Argument , *tmp; 
    struct sockaddr_in SC_serv_addr , CC_cli_addr , CD_cli_addr ; 

    fd_set read_fds;
    int fdmax, nready;
    pid_t childpid;
    int n;

    /* TCP Socket Creation */
    // Creating socket file descriptor
    if ( (S1_sockfd = socket( AF_INET, SOCK_STREAM, 0)) == 0 )
    {   
        perror( RED "Control Socket Creation Failure " WHITE ) ; 
        exit(EXIT_FAILURE) ; 
    }
    enable = 1;

    // Forcefully attaching socket to the port 8080
    if ( setsockopt(S1_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)))
    {
        perror( RED "setsockopt(SO_REUSEADDR) Failed " WHITE); 
    }

    memset(&SC_serv_addr, 0, sizeof(SC_serv_addr));
    SC_serv_addr.sin_family      = AF_INET ; 
    SC_serv_addr.sin_addr.s_addr = INADDR_ANY ; 
    SC_serv_addr.sin_port        = htons(PORT_X) ; 
    SC_serv_len                  = sizeof(SC_serv_addr) ; 

    // Forcefully attaching socket to the port 8080
    if(bind ( S1_sockfd , (struct sockaddr *) &SC_serv_addr , SC_serv_len ) < 0 )
    {
        perror( RED "Unable to Bind Control Socket to Local Address " WHITE ) ; 
        exit(EXIT_FAILURE) ; 
    }
    printf("Bind done\n");
    
    if (listen( S1_sockfd, 10) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

    // Data receiving timeout
    struct timeval tv;
    tv.tv_sec = 600;
    tv.tv_usec = 0;

    FD_ZERO(&read_fds);
    fdmax = S1_sockfd + 1;

    while(1)
    {   
        printf("Connection established\n");
        CC_cli_len = sizeof(CC_cli_addr) ;  

        FD_SET(S1_sockfd, &read_fds);
        nready = select(fdmax, &read_fds, NULL, NULL, &tv);
        if(nready == 0) {
            printf("2$timeout\n");
            exit(1);
        }

        // TCP socket is readable
        if(FD_ISSET(S1_sockfd, &read_fds)) {  
            if( (newS1_sockfd = accept( S1_sockfd, (struct sockaddr *) &CC_cli_addr, (socklen_t*)&CC_cli_len ) ) < 0 )
            {
                perror("Server Control Unable to Accept Incoming Connection "  ) ; 
                continue ;
            }
            printf("Accept done\n");

            if((childpid = fork()) == 0) {
                close(S1_sockfd);
                int commandno = 1;
            
                printf("\nServer Running.... \n\n") ;
                while(1) // This is waiting for the completion of the first command sequence. 
                {         
                    printf("Entering while\n");   
                    clearBuf(buffer);
                    if ( (recv_bytes = recv( newS1_sockfd , buffer , MAX_SIZE , 0 )) < 0 ) 
                    {
                        perror(RED "Command Receive Error " WHITE) ; 
                        continue ;
                    }
                    printf("Command is:%s\n",buffer);
                    sprintf( Command , "%s" , buffer ) ; 
                    
                    if(!strcmp(Command, "quit")) {
                        Error_Code = 421 ; 
                        fprintf(stderr, CYAN "%hi : %s\n" WHITE , Error_Code , "SUCCESS - Closing Connection.." );
                        int length1 = snprintf( NULL, 0, "%d", Error_Code );
                        char* str = malloc( length1 + 1 );
                        snprintf(str, length1 + 1, "%d", Error_Code );
                        send(newS1_sockfd,str,sizeof(str),0); 
                        free(str);
                        close(newS1_sockfd); 
                        exit(0);
                    }
                    if(commandno == 1)
                    {
                        Error_Code = parse_first(buffer) ;
                        loginSuccessFlag = 0;
                        commandno++; 
                    } 
                    else if(commandno == 2)
                    {
                        Error_Code = parse_first2(buffer) ; 
                        loginSuccessFlag = 0;
                        commandno++; 
                    }
                    else if(commandno == 3)
                    {

                        Error_Code = password_check(buffer) ;
                        printf("Error code is %d\n",Error_Code);
                        if(Error_Code == 500 || Error_Code == 600)
                        {
                            commandno = 1;
                        }
                        else loginSuccessFlag = 1;
                        commandno++;   
                    }
                    else if(commandno == 4 && loginSuccessFlag)
                    { 
                        char *Command_Name;
                        char Command[MAX_SIZE+1];
                        int code1;
                        strcpy(Command, buffer);
                        Command_Name = strtok(buffer, " ");
                        printf("Command name is %s\n", Command_Name);
                        code1 = parse(Command_Name);
                        if(code1 == 2)
                            Error_Code = change_directory(Command);
                        if(code1 == 8)
                            directory_listing(newS1_sockfd);
                        if(code1 == 4) {
                            val = put(Command, newS1_sockfd);
                            printf("val = %d\n", val);
                            continue;
                        }
                        if(code1 == 3)
                            get(Command, newS1_sockfd);
                        // commandno++; 
                    }
                    printf("Error code is %d\n",Error_Code);
                    int length = snprintf( NULL, 0, "%d", Error_Code );
                    char* str = malloc( length + 1 );
                    snprintf( str, length + 1, "%d", Error_Code );
                    send( newS1_sockfd , str , sizeof(str),0 ) ; 
                    free(str);
                    printf("Command no: %d\n",commandno); 
                    if(commandno > 4)
                    {
                        if( code == 5 )
                        {
                            Error_Code = 421 ; 
                            fprintf(stderr, CYAN "%hi : %s\n" WHITE , Error_Code , "SUCCESS - Closing Connection.." );
                            int length1 = snprintf( NULL, 0, "%d", Error_Code );
                            char* str = malloc( length1 + 1 );
                            snprintf(str, length1 + 1, "%d", Error_Code );
                            send(newS1_sockfd,str,sizeof(str),0); 
                            free(str);
                            close(newS1_sockfd); 
                            exit(0);
                        }         
                    }
                }
                if(stop)
                {   
                    close(newS1_sockfd) ;
                    exit(0);
                } 
            }
            close(newS1_sockfd) ; 
        }
    }
}