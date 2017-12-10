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
	int socket_descriptor, port, err, bytes_received, byte_cnt, bytes_header;
	unsigned int rcv_file_size;
	unsigned short rcv_len_file_name;
	struct sockaddr_in dest_addr;
	char *input_addr, *rcv_file_name, *path_name_buffer, hash_512[SHA512_DIGEST_LENGTH], *hash_512_string, rcv_hash_512_string[129];
	unsigned char *send_buffer[BUFFER_SIZE_MTU_PPPoE], rcv_buffer[BUFFER_SIZE_MTU_PPPoE], *file_buffer;
	socklen_t addrlen;
	FILE *fp;

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

	// receive header
	bytes_received = read(socket_descriptor, rcv_buffer, BUFFER_SIZE_MTU_PPPoE);
	memcpy(&rcv_len_file_name, rcv_buffer, 2);
	rcv_file_name = malloc(rcv_len_file_name+1);
	memcpy(rcv_file_name, rcv_buffer+2, rcv_len_file_name);
	strcat(rcv_file_name, "\0");
	printf(filename_str, rcv_file_name);

	memcpy(&rcv_file_size, rcv_buffer+2+rcv_len_file_name, 4);
	printf(filesize_str, rcv_file_size);

	bytes_header = 2 + rcv_len_file_name + 4 + 1; // 1 more...


	// receive data
	file_buffer = malloc(rcv_file_size);
	byte_cnt = 0;

	while(byte_cnt < rcv_file_size)
	{
		if(rcv_file_size - byte_cnt < BUFFER_SIZE_MTU_PPPoE)
		{
			bytes_received = read(socket_descriptor, rcv_buffer, rcv_file_size - byte_cnt);
			memcpy(file_buffer + byte_cnt, rcv_buffer, rcv_file_size - byte_cnt);
			byte_cnt = byte_cnt + bytes_received;
		} else
		{
			bytes_received = read(socket_descriptor, rcv_buffer, BUFFER_SIZE_MTU_PPPoE);
			memcpy(file_buffer + byte_cnt, rcv_buffer, BUFFER_SIZE_MTU_PPPoE);
			byte_cnt = byte_cnt + bytes_received;
		}
	}

	printf("received file (%d bytes)...\n", byte_cnt);

	// store data in file
	path_name_buffer = malloc(strlen(rcv_file_name) + 10);
	strncpy(path_name_buffer, "received/", 9);
	strncat(path_name_buffer, rcv_file_name, strlen(rcv_file_name));

	fp = fopen(path_name_buffer, "w+");
	if(fp == NULL)
	{
		printf("ERROR: creating file '%s' failed!\n", path_name_buffer);
	}
	fwrite(file_buffer, sizeof(char), rcv_file_size, fp);
	printf("created: %s...\n", path_name_buffer);
	fclose(fp);

	// SHA512
	SHA512(file_buffer, rcv_file_size, hash_512);
	hash_512_string = create_sha512_string(hash_512);
	printf(receiver_sha512, hash_512_string);

	bytes_received = read(socket_descriptor, rcv_hash_512_string, 129);
	if(strcmp(hash_512_string, rcv_hash_512_string) == 0)
	{
		printf(SHA512_OK);
		printf("sending result of sha comparison to sender (%c)...\n", SHA512_CMP_OK);
		write(socket_descriptor, &SHA512_OK, 1);

	} else
	{
		printf(SHA512_ERROR);
		printf("sending result of sha comparison to sender (%c)...\n", SHA512_CMP_ERROR);
		write(socket_descriptor, &SHA512_CMP_ERROR, 1);
	}

	free(file_buffer);
	free(path_name_buffer);
	free(rcv_file_name);
	close(socket_descriptor);

}