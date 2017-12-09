#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include"Aufgabe2.h"

void init_addr(struct sockaddr_in *addr, int port)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //host to network short encoding
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
}

int file_exists(char *fname)
{
  if(access(fname, F_OK ) != -1)
  {
    return 1;
  } else
  {
    return 0;
  }
}

int main(int argc, char *argv[])
{
	int socket_descriptor, err, res, newsock, flen;
	struct sockaddr_in sender_addr, client_addr;
	socklen_t addrlen;
	char msg[64];

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
      printf("ERROR: please provide a valid number for a port\n");
      return 0;
  	}

  	// extract file path
  	char *filepath;
	filepath = argv[2];
    if(!file_exists(filepath))
	{
		printf("ERROR: please give a valid file path\n");
	   	return 0;
	}

	printf("start sender...\n");


	// create socket
	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_descriptor < 0)
	{
		printf("ERROR: socket initialization failed!\n");
		return 0;
	}

	// init address
	init_addr(&sender_addr, port);

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

	newsock = accept(socket_descriptor, (struct sockaddr *) &client_addr, &addrlen);
	if(newsock<0)
	{
		printf("ERROR: accepting failed!\n");
		return 0;
	}

	printf("Received connection from %s!\n", inet_ntoa(client_addr.sin_addr));

	read(socket_descriptor, &msg, &flen);
	printf("msg: %s\n", msg);
}




