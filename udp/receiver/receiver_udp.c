#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>

#include"../Aufgabe2.h"

void init_addr(struct sockaddr_in* addr, int port, char *server_addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //host to network short encoding
	addr->sin_addr.s_addr = inet_addr(server_addr);
}

int main(int argc, char *argv[])
{
    //----------------------------------
    //--- Check user input
    if(argc < 3)
    {
        printf("Please provide address and port\n");
        return 0;
    }

    char *server_addr;
    server_addr = argv[1];

    int port;
    port = atoi(argv[2]);
    if(port == 0)
    {
        printf("Please provide a number for port\n");
        return 0;
    }

    //----------------------------------
    int socket_descriptor, dest_descriptor, err, socklen;
    struct sockaddr_in dest_addr;

    //--- Initiate a socket
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_descriptor < 0)
    {
        perror("Socket was not created: ");
    }
    printf("Socket is initialized\n");

    //--- Fill in sockaddr_in
    init_addr(&dest_addr, port, server_addr);

    //--- Sending
    unsigned char msg[2048];
		memcpy(msg, &REQUEST_T, sizeof(unsigned char));
    socklen = sizeof(struct sockaddr_in);
    err = sendto(socket_descriptor, msg, sizeof(msg) + 1, 0, (struct sockaddr*) &dest_addr, socklen);
    if(err<0)
    {
      perror("Send error: ");
    }
		printf("Request sent\n");

		//--- Receiving
		unsigned char buff[2048];
		printf("Waiting for response...\n");
		err = recvfrom(socket_descriptor, buff, sizeof(buff) + 1, 0, (struct sockaddr *) &dest_addr,(socklen_t*) &socklen);
		if(err<0)
		{
			perror("Connection error: ");
		}
		else if(err == 0)
		{
			printf("Empty message received\n");
		}

		unsigned char typID = buff[0];
		if(typID == HEADER_T)
		{
			printf("Saving file...\n");
			unsigned short namelen = buff[sizeof(unsigned char)];
			char* name = &buff[sizeof(unsigned char) + sizeof(unsigned short)];

			printf("YAHOOO %s\n",name);

			printf("%hhu\n",buff[0]);
			printf("%hhu\n",buff[1]);
			printf("%hhu\n",buff[2]);
			printf("%hhu\n",buff[3]);
			printf("%hhu\n",buff[4]);
			printf("%hhu\n",buff[5]);
		}

}
