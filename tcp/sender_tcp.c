#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <libgen.h>
#include"Aufgabe2.h"

int main(int argc, char *argv[])
{
	int socket_descriptor, err, res, newsock_destriptor, byte_cnt;
	struct sockaddr_in sender_addr, client_addr;
	socklen_t addrlen;
	char buffer[BUFFER_SIZE_MTU_PPPeE];

	// check number of arguments
	if(argc!=3)
	{
		printf("invalid number of args. Expected 2, received %d.\n", argc-1);
		return 0;
	}

	// extract port from arguments
	int port;
  	port = atoi(argv[1]);
  	if(port < 0 || port > 65536)
  	{
  	  printf(port_error, argv[1]);
      return 0;
  	}

  	// extract file path
  	char *filepath;
	filepath = argv[2];
    if(!file_exists(filepath))
	{
		printf("ERROR: the file '%s' does not exist. Please provide a valid file path.\n", filepath);
	   	return 0;
	}

	printf("start sender...\n");

	printf("file name: %s\n", get_file_name(filepath));


	// create socket
	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_descriptor < 0)
	{
		printf("ERROR: socket initialization failed!\n");
		return 0;
	}

	// init address
	init_addr_sender(&sender_addr, port);

	// binding
	err = bind(socket_descriptor, (struct sockaddr *) &sender_addr, sizeof(struct sockaddr_in));
	if(err < 0)
	{
		printf("ERROR: binding failed!\n");
		return 0;
	}

	res = listen(socket_descriptor, 5);
	if(res<0)
	{
		printf("ERROR: listening failed!\n");
		return 0;
	}

	addrlen = sizeof(struct sockaddr_in);

	newsock_destriptor = accept(socket_descriptor, (struct sockaddr *) &client_addr, &addrlen);
	if(newsock_destriptor<0)
	{
		printf("ERROR: accepting failed!\n");
		return 0;
	}

	printf("Received connection from %s!\n", inet_ntoa(client_addr.sin_addr));
	byte_cnt = read(newsock_destriptor, buffer, BUFFER_SIZE_MTU_PPPeE);
	if(byte_cnt <0)
	{
		printf("ERROR: receiving failed!\n");
		return 0;
	}
	printf("received %d bytes\n", byte_cnt);
	printf("msg: %s\n", buffer);

	write(newsock_destriptor, "got it!", strlen("got it") + 1);

	close(newsock_destriptor);	
	close(socket_descriptor);

}




