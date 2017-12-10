#include<arpa/inet.h>
#include<netinet/in.h>
#include<openssl/sha.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>

#include"Aufgabe2.h"

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
    printf("\nSocket is initialized\n");

    //--- Fill in sockaddr_in
    init_addr(&dest_addr, port, server_addr);

		//----------------------------------
    //--- Sending request
    unsigned char msg[2048];
		memcpy(msg, &REQUEST_T, sizeof(unsigned char));
    socklen = sizeof(struct sockaddr_in);
    err = sendto(socket_descriptor, msg, sizeof(msg) + 1, 0, (struct sockaddr*) &dest_addr, socklen);
    if(err<0)
    {
      perror("Send error: ");
    }
		printf("\nRequest sent\n");

		//----------------------------------
		//--- Receiving header
		unsigned char buff[2048];
		printf("\nWaiting for response...\n");
		err = recvfrom(socket_descriptor, buff, sizeof(buff) + 1, 0, (struct sockaddr *) &dest_addr,(socklen_t*) &socklen);
		if(err<0)
		{
			perror("Connection error: ");
		}
		else if(err == 0)
		{
			printf("Empty message received\n");
		}

		//----------------------------------
		// Receiving the files
		unsigned char typID = buff[0];
		if(typID == HEADER_T)
		{
			//--- Reading the header
			printf("\nReceived header:\n");

			unsigned short namelen;
			memcpy(&namelen, buff + sizeof(unsigned char), sizeof(unsigned short));

			char* name = (char*)malloc(sizeof(char)*(namelen+1));
			memcpy(name, buff + sizeof(unsigned char) + sizeof(unsigned short), namelen);
			strcat(name, "\0");
			printf(filename_str,name);

			unsigned int datalen;
			memcpy(&datalen, buff + sizeof(unsigned char) + sizeof(unsigned short) + namelen, sizeof(unsigned int));
			printf(filesize_str, datalen);
			printf("Message size in bytes: %d\n", err);

			FILE *file = fopen("received/test3.tar.gz","wb+");
			if(!file)
			{
				perror("File wasn't created");
			}
			// Clean up
			free(name);

			//--- Receiving the packages
			printf("\nPreparing to receive packages:\n");
			unsigned int seq;
      unsigned int referrence = 0;
			unsigned char pkg_buff[2048];
			do
			{
				//TODO: Clean the buffer
				err = recvfrom(socket_descriptor, pkg_buff, sizeof(pkg_buff) + 1, 0, (struct sockaddr *) &dest_addr,(socklen_t*) &socklen);
				if(err<0)
				{
					perror("Connection error: ");
				}
				else if(err == 0)
				{
					printf("Empty message received\n");
				}

				typID = pkg_buff[0];
				if(typID == DATA_T)
				{
					memcpy(&seq, pkg_buff + sizeof(unsigned char), sizeof(unsigned int));
          if(seq != referrence++)
          {
            printf(order_error, seq, referrence);
          }
					unsigned int pkg_size = get_pkg_size(seq, datalen);
					printf("Receiving %d-st package of size %d\n", seq, pkg_size);
					unsigned char file_buff[pkg_size];
					memcpy(file_buff, pkg_buff + sizeof(unsigned char) + sizeof(unsigned int), pkg_size);
					fwrite(file_buff, sizeof(unsigned char), pkg_size, file);
				}
			}
			while(seq != datalen/MTU);

			fseek(file, 0, SEEK_END);
		  unsigned int size = ftell(file);
		  fclose(file);

			printf("Size of the created file: %d\n", size);

			//----------------------------------
	    //--- SHA Value
			// Receiving the Message
			err = recvfrom(socket_descriptor, buff, sizeof(buff), 0, (struct sockaddr *) &dest_addr,(socklen_t*) &socklen);
			if(err<0)
			{
				perror("Connection error: ");
			}
			else if(err == 0)
			{
				printf("Empty message received\n");
			}

			typID = buff[0];
			if(typID == SHA512_T)
			{
				printf("\nReceiving SHA\n");
				unsigned char hash_512[SHA512_DIGEST_LENGTH];
				memcpy(hash_512, buff + sizeof(unsigned char), SHA512_DIGEST_LENGTH);
				char *hash = create_sha512_string(hash_512);

				// Calculating SHA
				printf("\nCalculating SHA\n");
				file = fopen("./received/test3.tar.gz","rb");
				unsigned char fbuff[datalen];
				fread(fbuff, sizeof(char), datalen, file);
				SHA512(fbuff, datalen, hash_512);
				char *hash_cal = create_sha512_string(hash_512);
				printf(receiver_sha512, hash_cal);

				//--- Preparing the answer
				memcpy(buff, &SHA512_CMP_T, sizeof(unsigned char));
				int comp = strcmp(hash, hash_cal);
				if(comp == 0)
				{
					printf(SHA512_OK);
					memcpy(buff + sizeof(unsigned char), &SHA512_CMP_OK, sizeof(char));
				}
				else
				{
					printf(SHA512_ERROR);
					memcpy(buff + sizeof(unsigned char), &SHA512_CMP_ERROR, sizeof(char));
				}
				err = sendto(socket_descriptor, buff, sizeof(msg) + 1, 0, (struct sockaddr*) &dest_addr, socklen);
		    if(err<0)
		    {
		      perror("Send error: ");
		    }
			}

		}
		printf("\nFile contains: \n");
		system("cat received/test3.tar.gz");
}
