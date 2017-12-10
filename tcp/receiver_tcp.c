#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<openssl/sha.h>
#include"Aufgabe2.h"

int main(int argc, char *argv[])
{
	int socket_descriptor, port, err, byte_cnt;
	unsigned int rcv_file_size;
	unsigned short rcv_len_file_name;
	struct sockaddr_in dest_addr;
	char *input_addr, *rcv_file_name;
	unsigned char *send_buffer[BUFFER_SIZE_MTU_PPPoE], rcv_buffer[BUFFER_SIZE_MTU_PPPoE];
	socklen_t addrlen;

	// check number of arguments
	if(argc!=3)
	{
		printf("invalid number of args. Expected 2, received %d.\n", argc-1);
		return 0;
	}

	// extract port from arguments
  	port = atoi(argv[2]);
  	if(port < 0 || port > 65536)
  	{
  	  printf(port_error, argv[2]);
      return 0;
  	}

	// extract addr
	input_addr = argv[1];
    init_addr_receiver(&dest_addr, port, input_addr);


	printf("start receiver...\n");

	// create socket
	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_descriptor < 0)
	{
		printf("ERROR: socket initialization failed!\n");
		return 0;
	}

	// connect to sender
	addrlen = sizeof(struct sockaddr_in);
	err = connect(socket_descriptor, (struct sockaddr *) &dest_addr, addrlen);
	if(err < 0){
		printf("ERROR: connecting failed!\n");
		return 0;
	}

	// send msg
	//strcpy(send_buffer,"hello, world!");
	//printf("sending msg '%s'...\n", send_buffer);
	//byte_cnt = write(socket_descriptor, send_buffer, strlen(send_buffer)+1);

	byte_cnt = read(socket_descriptor, rcv_buffer, BUFFER_SIZE_MTU_PPPoE);
	memcpy(&rcv_len_file_name, rcv_buffer, 2);
	printf("received header...\n");
	printf("length of file name: %d\n", rcv_len_file_name);
	rcv_file_name = malloc(rcv_len_file_name+1);
	memcpy(rcv_file_name, rcv_buffer+2, rcv_len_file_name);
	strcat(rcv_file_name, "\0");
	printf(filename_str, rcv_file_name);

	memcpy(&rcv_file_size, rcv_buffer+2+rcv_len_file_name, 4);

	printf("length of file: %d\n", rcv_file_size);

	


	free(rcv_file_name);
	close(socket_descriptor);

}