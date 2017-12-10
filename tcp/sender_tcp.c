#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<libgen.h>
#include<openssl/sha.h>
#include"Aufgabe2.h"

int main(int argc, char *argv[])
{
	int socket_descriptor, err, res, newsock_destriptor, bytes_sent, byte_cnt;
	unsigned int file_size;
	unsigned short len_file_name;
	struct sockaddr_in sender_addr, client_addr;
	socklen_t addrlen;
	char hash_512[SHA512_DIGEST_LENGTH], *hash_512_string, rcv_sha_comp;
	unsigned char send_buffer[BUFFER_SIZE_MTU_PPPoE], rcv_buffer[BUFFER_SIZE_MTU_PPPoE], *file_buffer, *msg, *file_name, *archive_file_name;
	FILE *fp;	

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
  	char *file_path;
	file_path = argv[2];
    if(!file_exists(file_path))
	{
		printf("ERROR: the file '%s' does not exist. Please provide a valid file path.\n", file_path);
	   	return 0;
	}

	printf("start sender...\n");

	file_name = get_file_name(file_path);
	archive_file_name = get_archive_name(file_name);

	printf("archive %s...\n", file_path);
	archive(file_name, file_path);

	printf(filename_str, archive_file_name);

	file_size = get_file_size(archive_file_name);
	printf(filesize_str, file_size);
	file_buffer = malloc(file_size);
	fp = fopen(archive_file_name, "rb");
	if(fp==NULL)
	{

		printf("ERROR: could not open file: %s\n", archive_file_name);
		fclose(fp);
		return 0;
	}

	fread(file_buffer, sizeof(char), file_size, fp);
	SHA512(file_buffer, file_size, hash_512);

	hash_512_string = create_sha512_string(hash_512);
	printf(sender_sha512, hash_512_string);

	fclose(fp);

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
	
	// send header
	len_file_name = strlen(archive_file_name);
	memcpy(send_buffer, &len_file_name, 2);
	memcpy(send_buffer + 2, archive_file_name, len_file_name);
	memcpy(send_buffer + 2 + len_file_name, &file_size, 4);
	printf("send header...\n");
	bytes_sent = write(newsock_destriptor, send_buffer, BUFFER_SIZE_MTU_PPPoE);
	
	// send file
	printf("send file...\n");
	byte_cnt = 0;
	while(byte_cnt<file_size)
	{
		if(file_size - byte_cnt < BUFFER_SIZE_MTU_PPPoE)
		{
			memcpy(send_buffer, file_buffer + byte_cnt, file_size - byte_cnt);
			bytes_sent = write(newsock_destriptor, send_buffer, file_size - byte_cnt);
			byte_cnt = byte_cnt + bytes_sent;
		} else
		{
			memcpy(send_buffer, file_buffer + byte_cnt, BUFFER_SIZE_MTU_PPPoE);
			bytes_sent = write(newsock_destriptor, send_buffer, BUFFER_SIZE_MTU_PPPoE);
			byte_cnt = byte_cnt + bytes_sent;
		}
	}
	printf("file (%d bytes) successfully sent...\n", byte_cnt);

	// send sha512
	memcpy(send_buffer, hash_512_string, 129);
	bytes_sent = write(newsock_destriptor, send_buffer, 129);
	printf("send sha-512-String (%d bytes)...\n", bytes_sent);

	// receive result of sha comparison
	bytes_sent = read(newsock_destriptor, &rcv_sha_comp, 1);
	printf("received result of sha-512-String comparison...\n");
	if(rcv_sha_comp == SHA512_CMP_OK)
	{
		printf(SHA512_OK);
	} else if(rcv_sha_comp == SHA512_CMP_ERROR)
	{
		printf(SHA512_ERROR);
	} else
	{
		printf("FAIL!!\n");
	}


	close(newsock_destriptor);	
	close(socket_descriptor);

	free(hash_512_string);
	free(archive_file_name);
	free(file_buffer);
}




