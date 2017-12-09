#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <openssl/sha.h>
#include"Aufgabe2.h"

int main(int argc, char *argv[])
{
	int socket_descriptor, port, err, byte_cnt;
	struct sockaddr_in dest_addr;
	char *input_addr, send_buffer[BUFFER_SIZE_MTU_PPPeE], rcv_buffer[BUFFER_SIZE_MTU_PPPeE];
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

	read(socket_descriptor, rcv_buffer, BUFFER_SIZE_MTU_PPPeE);

	printf("received: %s\n", rcv_buffer);

	close(socket_descriptor);

}