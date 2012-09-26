//Sockets AP

#include "frft_socket_api.h"

//char sock_send_buffer[MAX_SOCK_BUFF];
//char sock_recv_buffer[MAX_SOCK_BUFF];

struct sockaddr_in  get_self_server_address(int portno)
{

        struct sockaddr_in serv_addr;
        //int portno = RECV_PORTNO;

        bzero((char *)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);

        return serv_addr;
}

struct sockaddr_in get_server_address(char *server_ip,int portno)
{

        struct sockaddr_in serv_addr;
        struct hostent *server;
        server = gethostbyname(server_ip); //CHANGE THIS WHILE RUNNING
        if (server == NULL)
        {
                printf("ERROR: cannot get server name\n");
                //return -1;
                exit(1); // can be changed later
        }

//      printf("Got server address\n\n\n");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
                (char *)&serv_addr.sin_addr.s_addr,
                 server->h_length);
        serv_addr.sin_port = htons(portno);

        return serv_addr;

}

int frft_create_socket()
{
        // Creates a socket and returns a file descriptor
        int sock_fd;
	int n = MAX_SOCK_RECVBUFF;
	int socket_reuse_on = 1;
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock_fd < 0)
        {
                printf("Socket API: Error in opening socket\n");
                //exit(0);
                return -1; // replace this -1 by error symbol
        }
	//may need to change afterwards to a function to add more functionality
	setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,&n,sizeof(n));
	setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&socket_reuse_on,sizeof(socket_reuse_on));
        return sock_fd;
}
                        
int frft_bind_socket(int sock_fd, struct sockaddr_in recv_addr)
{
        // Bind a UDP socket to receiver's address
        int bind_result;
        bind_result = bind(sock_fd, (struct sockaddr *) &recv_addr,
                           sizeof(recv_addr));
        
        if(bind_result < 0)
        {
                printf("Socket API: Error on binding\n");
                return -1; // replace this -1 by error symbol
        }
        return bind_result;
}


int frft_send_UDP(int sock_fd, struct sockaddr_in recv_addr,char *buffer,int buffer_len)
{

        // UDP sendto API
        int sendto_result;
        //char *buff_send = sock_send_buffer;
	// might want to do bzero here at later stage
	//memcpy((void *) buff_send, (void *) buffer, buffer_len);	
//	printf("socket buff : %s\n\n\n",buff_send);
        sendto_result = sendto(sock_fd,buffer,buffer_len,0,(struct sockaddr *)&recv_addr,sizeof(recv_addr));

        if (sendto_result < 0)
        {
                printf("Socket API: Error in sendto\n");
                return -1; //replace this -1 by error symbol
        }
        return sendto_result;
}

int frft_recv_UDP(int sock_fd, struct sockaddr_in *sender_addr,char *buffer,int buffer_len)// have a option of passing the sender's address len as reference
{
        // UDP recvfrom API
        int recv_result;
        socklen_t sender_addr_len;
	//struct sockaddr_in sender_addr1;
	//char *buff_recv = recv_buffer;
	//bzero(buff_recv,MAX_SOCK_BUFF);
	sender_addr_len = sizeof(struct sockaddr_in);
	recv_result = recvfrom(sock_fd,buffer,buffer_len,0,(struct sockaddr *)sender_addr,&sender_addr_len);

	if(recv_result < 0)
	{
		printf("Socket API: Error in recvfrom\n");
		perror("recvfrom\n");
		return -1; //replace this -1 by error symbol
	}
//	printf("RECV BUFF size and length : %d\n",recv_result);
//	printf("RECV BUFF: %s\n\n\n",buff_recv);
//	bzero(buffer,MAX_SOCK_BUFF);
	//memcpy((void *) buffer,(void *) buff_recv,recv_result);
//	memmove((void *)recv_buffer,(void *)buff_recv,recv_result);
	//memcpy(buffer,buff_recv,recv_result-1);
//	frft_file_write(recv_file,buff_recv,MAX_SOCK_BUFF);

	return recv_result;
}




