//Includes for sockets
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h> 

#define MAX_SOCK_BUFF 1452
#define MAX_SOCK_RECVBUFF 1024*1024*1024
#define RECV_PORTNO 3333
#define SENDER_ACK_PORTNO 4444
#define MAX_NUMBER_OF_PACKETS 1024*256
#define SLEEP_TIME 20


typedef struct payload {
     unsigned int seq_num;
     unsigned int data_size;
     char data[MAX_SOCK_BUFF];
}udp_payload;


/*
extern char send_buffer[MAX_SOCK_BUFF];
extern char recv_buffer[MAX_SOCK_BUFF];
extern char file_read_buffer[MAX_SOCK_BUFF];
extern char file_write_buffer[MAX_SOCK_BUFF];
extern char sock_send_buffer[MAX_SOCK_BUFF];
extern char sock_recv_buffer[MAX_SOCK_BUFF];
extern FILE *send_file,*recv_file;
*/

/*
int frtf_create_socket();
int frft_bind_socket(int sock_fd, struct sockaddr_in recv_addr);
int frft_send_UDP(int sock_fd, struct sockaddr_in recv_addr,char *buffer,int buffer_len);
int frft_recv_UDP(int sock_fd, struct sockaddr_in *sender_addr,char *buffer,int buffer_len);
int frft_file_read(FILE *sendfile,char *buffer,int buffer_len)
int frft_file_write(FILE *recvfile,char *buffer, int buffer_len)
*/

struct sockaddr_in  get_self_server_address(int portno);
struct sockaddr_in get_server_address(char *server_ip,int portno);


