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
	int socket_descriptor, err, res, newsock_destriptor, byte_cnt;
	unsigned int file_size;
	struct sockaddr_in sender_addr, client_addr;
	socklen_t addrlen;
	char hash_512[SHA512_DIGEST_LENGTH], *hash_512_string, *file_name, *archive_file_name;
	unsigned char send_buffer[BUFFER_SIZE_MTU_PPPoE], rcv_buffer[BUFFER_SIZE_MTU_PPPoE], *file_buffer, *msg;
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

	printf(filename_str, file_name);

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

	//printf("CP %s\n", get_archive_name(get_file_name(file_path)));
	
	//TODO
	/**fp = fopen(archive_file_name, "r");
	if(fp==NULL)
	{

		printf("ERROR: could not open file: %s\n", archive_file_name);
		fclose(fp);
		free(archive_file_name);
		return -1;
	}

	printf(sender_sha512, );*/


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
	//byte_cnt = read(newsock_destriptor, buffer, BUFFER_SIZE_MTU_PPPeE);
	//if(byte_cnt <0)
	//{
	//	printf("ERROR: receiving failed!\n");
	//	return 0;
	//}
	//printf("received %d bytes\n", byte_cnt);
	//printf("msg: %s\n", buffer);



	write(newsock_destriptor, "connection established!", strlen("connection established!") + 1);

	//send_buffer

	close(newsock_destriptor);	
	close(socket_descriptor);

	free(hash_512_string);
	free(archive_file_name);
	free(file_buffer);
}




