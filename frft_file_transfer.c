#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "frft_socket_api.h"
#include "pthread.h"

char file_name[50];
char serverIP[30];

char static ack_bit_array[MAX_NUMBER_OF_PACKETS];
pthread_mutex_t ack_bit_array_mutex = PTHREAD_MUTEX_INITIALIZER;
int number_of_packets;

int extractbit(char data, int i) {
    return (data >> i) & 0x01;
}


int sender_function()
{
	udp_payload send_payload;
	FILE *send_file;
	int send_sock;
	int result;
	int bit,flag=0;

	struct sockaddr_in serv_addr;
    time_t start_seconds,end_seconds;
 	unsigned int start_seq_num=0,counter;
	start_seconds = time(NULL);

	send_file = fopen(file_name,"r");
	if(send_file == NULL)
		printf("Error: opening file\n");

	fseek(send_file, 0, SEEK_END); // seek to end of file
	int size_send_file = ftell(send_file); // get current file pointer
	number_of_packets = size_send_file/MAX_SOCK_BUFF + ((size_send_file%MAX_SOCK_BUFF > 0) ? 1:0);
	fseek(send_file, 0, SEEK_SET);
	//printf(" number_of_packets %d \n",number_of_packets);

	send_sock = frft_create_socket();
	
	serv_addr = get_server_address(serverIP,RECV_PORTNO);

//	while(!feof(sendfile))
	for(counter =0 ; counter<number_of_packets; counter++)
	{
		send_payload.seq_num = start_seq_num++;

		result = frft_file_read(send_file,(void *)send_payload.data,MAX_SOCK_BUFF);
		if(result < 0)
		{	
			printf("Error : Problem in read\n");
			return -1;
		}
		send_payload.data_size = result;

//		printf("read from file %d\n\n\n",counter);
		result = frft_send_UDP(send_sock,serv_addr,(void *)&send_payload,sizeof(send_payload));
		if(result < 0)
        	{ 
        		printf("Error : Problem in send\n");
        	        return -1;
        	}
//		printf("udp packet send %d\n", send_payload.seq_num);
//		usleep(SLEEP_TIME);
	}
	

	//printf("file tranfer complete...now yielding");

	//int retrans = retransmit();
	//sleep(1);


	//printf("now resending");
	/*int k;
	for(k = 0; k < number_of_packets/8; k++) {
								printf("  %u  ", *(unsigned char*) &ack_bit_array[k]);
									}
	*/

	while(1) {
		flag=0;
		for(counter =0 ; counter<number_of_packets; counter++)
				{
					//printf("\n Checking packet %d",counter);
					pthread_mutex_lock( &ack_bit_array_mutex );

					bit = extractbit(ack_bit_array[counter/8],counter%8);

					pthread_mutex_unlock( &ack_bit_array_mutex );
					//printf("Resending %d  %d %d ", counter, bit, ack_bit_array[counter/8]);

					if(!bit)
					{
						flag = 1;
						send_payload.seq_num = counter;

						int error_fseek = fseek(send_file,counter*MAX_SOCK_BUFF,SEEK_SET);
						if(error_fseek != 0)
								printf("send file fseek Error");

						result = frft_file_read(send_file,(void *)send_payload.data,MAX_SOCK_BUFF);
						if(result < 0)
						{
							printf("Error : Problem in read\n");
							return -1;
						}
						send_payload.data_size = result;
				//		printf("read from file %d\n\n\n",counter);
						result = frft_send_UDP(send_sock,serv_addr,(void *)&send_payload,sizeof(send_payload));
						if(result < 0)
							{
								printf("Error : Problem in send\n");
									return -1;
							}
				//		printf("udp packet send\n\n\n");
						//usleep(SLEEP_TIME);
					}
				}
		if(flag == 0)
			break;
	}
	fclose(send_file);

	//send_payload.seq_num = 9999999;
	for(counter=0; counter<5 ;counter++)
		result = frft_send_UDP(send_sock,serv_addr,(void *)&send_payload,0);

	close(send_sock);
	end_seconds = time(NULL);
	
	//printf("\nStart time %d\nEnd time %d\n",(int)start_seconds,(int)end_seconds);
	printf("Total time for sending %d\n",((int)end_seconds - (int)start_seconds));

	pthread_exit(0);
}

int ack_receiver_function()
{
	int ack_recv_sock;
	int result;
	struct sockaddr_in ack_serv_addr;
	struct sockaddr_in ack_sender_addr;
	//int recv_flag = 0;
	unsigned int ack_seq_num;
	int counter;

	ack_recv_sock = frft_create_socket();
	
	ack_serv_addr = get_self_server_address(SENDER_ACK_PORTNO);
	
	result = frft_bind_socket(ack_recv_sock,ack_serv_addr);

	while(1)
	{
		
		result = frft_recv_UDP(ack_recv_sock,&ack_sender_addr,(unsigned int *)&ack_seq_num,sizeof(ack_seq_num));
		if(result < 0)
		{
			printf("Error : Problem in receive for ack\n");
			return -1;
		}

	//	printf(" ACK Sequence number : %d\n",ack_seq_num);

		else if(result != 0){
			pthread_mutex_lock( &ack_bit_array_mutex );

			ack_bit_array[ack_seq_num/8] |= 1 << ack_seq_num%8;

			pthread_mutex_unlock( &ack_bit_array_mutex );
		}


		else // i.e (ack_seq_num == cmp)
			{
				//recv_flag = 1;
			//	printf("Printing ack_bit_array");

				/*for(counter = 0; counter < number_of_packets/8; counter++) {
						printf("  %u  ", *(unsigned char*) &ack_bit_array[counter]);
							}*/

			//	printf("END of acks means end of transmission....hurray\n");
				break;
			}

	}
	close(ack_recv_sock);
	pthread_exit(0);
}	
	

int receiver_function()
{

	udp_payload recv_payload;

	int recv_sock;
	int ack_sender_sock;
	int result,i;
	struct sockaddr_in serv_addr;
	struct sockaddr_in sender_addr;
	time_t start_seconds,end_seconds;
	int flood,error_fseek;

	FILE *new_recv_file,*recv_file;

        start_seconds = time(NULL);

        //struct hostent *server;
        //int portno = RECV_PORTNO;
        recv_file = fopen(file_name,"w");
        if(recv_file == NULL)
                printf("Error: opening file\n");

        ack_sender_sock = frft_create_socket();

        recv_sock = frft_create_socket();


//	printf("Created socket\n\n\n");

	serv_addr = get_self_server_address(RECV_PORTNO);
        
	result = frft_bind_socket(recv_sock,serv_addr);
	//for(i = 0; i<number_of_packets; i++) // change after words
        //int cmp = 9999999;
        //i=0;
    while(1)
	{		
		//recvfile = fopen(file_name,"w");
		result = frft_recv_UDP(recv_sock,&sender_addr,(udp_payload *)&recv_payload,sizeof(recv_payload));
		if(result < 0)
                { 
                        printf("Error : Problem in receive\n");
                        return -1;
                }
		/*printf(" Sequence number : %d %d  ",recv_payload.seq_num, i);
		i++;*/


		if(result == 0){
			sender_addr.sin_port = htons(SENDER_ACK_PORTNO);

				//recv_payload.seq_num = 9999999;
				//result = frft_send_UDP(ack_sender_sock,sender_addr,(void *)&recv_payload.seq_num,sizeof(recv_payload.seq_num));
				for(i=0; i<5; i++)
					result = frft_send_UDP(ack_sender_sock,sender_addr,(void *)&recv_payload.seq_num,0);

			    fclose(recv_file);
			    break;

		}
	//	printf("Now writing\n");
		error_fseek = fseek(recv_file,recv_payload.seq_num*MAX_SOCK_BUFF,SEEK_SET);
		if(error_fseek != 0)
			printf("recv_file fseek Error");

//	    	printf("data received %d \n",i);	
		result = frft_file_write(recv_file,(void *)recv_payload.data,recv_payload.data_size);
		if(result < 0)
                {
	               printf("Error : Problem in write\n");
                       return -1;
                }
//		printf("data written %d\n\n",i);
		//fclose(recvfile);

		sender_addr.sin_port = htons(SENDER_ACK_PORTNO);

		for(flood =0; flood < 3; flood++) {
			result = frft_send_UDP(ack_sender_sock,sender_addr,(void *)&recv_payload.seq_num,sizeof(recv_payload.seq_num));
	        	if(result < 0)
               		{
                       printf("Error : Problem in sending the ack\n");
                       return -1;
               		}
		}

	}

    close(recv_sock);
    close(ack_sender_sock);
	end_seconds = time(NULL);

       // printf("\nStart time %d\nEnd time %d",(int)start_seconds,(int)end_seconds);
        printf("Total time for receiving \n %d",((int)end_seconds - (int)start_seconds));

}



int main(int argc, char* argv[])
{
	int server_flag = -1;
	int pid,err;
	pthread_t sender_thread, ack_receiver_thread;

	//char file_name[50];

	if(argc < 3)	
	{
		printf("Error in syntax. \n");
		printf("Usage -s  filename    OR \n");
		printf("Usage -c  filename ServerIPAddress\n");
		return -1;
	}


	if(strcmp(argv[1],"-s") == 0)
		server_flag = 1;
	else if(strcmp(argv[1],"-c") == 0)
		server_flag =  0;

	//file_len = strlen(argv[3]);
	strcpy(file_name,argv[2]);

	//printf("\n FILE NAME : %s\n",file_name);

	if(server_flag == 1)
	{
		//printf("\n CALLING RECEIVER");
		receiver_function();
	}
	else if(server_flag == 0) 
	{
		if(argc < 4)
			{
				printf("Error in syntax. Usage -c  filename ServerIPAddress\n");
				return -1;
			}

		strcpy(serverIP,argv[3]);
		//printf("\n CALLING SENDER");

		err = pthread_create(&ack_receiver_thread, NULL,(void *)&ack_receiver_function, NULL);
		if(err != 0) perror("Thread creation error");

		err = pthread_create(&sender_thread, NULL, (void *)&sender_function, NULL);
		if(err != 0) perror("Thread creation error");

		pthread_join(ack_receiver_thread, NULL);
		pthread_join(sender_thread, NULL);

		exit(0);

	}
	else
		printf("\n CALLING NOTHING");
	
}

