#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include"Aufgabe2.h"

void init_addr(struct sockaddr_in* addr, int port, char *sender_addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //host to network short encoding
	addr->sin_addr.s_addr = inet_addr(sender_addr);
}

int main(int argc, char *argv[])
{
	int socket_descriptor, port, err, byte_cnt;
	struct sockaddr_in dest_addr;
	char *input_addr, msg[64];
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
      printf("ERROR: please provide a valid number for a port\n");
      return 0;
  	}

	// extract addr
	input_addr = argv[1];
    init_addr(&dest_addr, port, input_addr);


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
	strcpy(msg,"hello, world!");
	printf("sending msg '%s'...\n", msg);
	byte_cnt = write(socket_descriptor, msg, strlen(msg)+1);

}