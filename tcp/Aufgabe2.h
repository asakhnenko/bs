#ifndef AUFGABE_2_H_
#define AUFGABE_2_H_

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h> // new

static const size_t BUFFER_SIZE_MTU_PPPeE = 1492;

static const char SHA512_CMP_OK = 0;
static const char SHA512_CMP_ERROR = -1;

static const unsigned char REQUEST_T = 0;
static const unsigned char HEADER_T = 1;
static const unsigned char DATA_T = 2;
static const unsigned char SHA512_T = 3;
static const unsigned char SHA512_CMP_T = 4;

static const char* const SHA512_OK 	        = "\x1b[32mSHA512 OK \x1b[0m\n";
static const char* const SHA512_ERROR 	    = "\x1b[31mSHA512 Error\x1b[0m\n";
static const char* const port_error 	    = "\x1b[31mInvalid Port (%s) \x1b[0m\n";
static const char* const address_error 	    = "\x1b[31mInvalid Address (%s) or Port (%s) \x1b[0m\n";
static const char* const packet_error       = "\x1b[31mInvalid Packet received \x1b[0m\n";
static const char* const order_error        = "\x1b[31mInvalid Packet Order: received %d, expected %d \x1b[0m\n";
static const char* const timeout_error      = "\x1b[31mTimeout reached, aborting..\x1b[0m\n";
static const char* const receiver_sha512    = "\x1b[34mReceiver SHA512: %s \x1b[0m\n";
static const char* const sender_sha512      = "\x1b[34mSender SHA512: %s \x1b[0m\n";
static const char* const filename_str 	    = "\x1b[33mFilename: %s \x1b[0m\n";
static const char* const filesize_str 	    = "\x1b[33mFilesize: %d bytes\x1b[0m\n";

static char* create_sha512_string(unsigned char* sha512) {
    char* result = (char*) malloc(129);
    int i;
    for(i = 0; i < 64; i++){
        sprintf(result+2*i,"%02x",*(sha512+i));
    }
    return result;
}

void init_addr_receiver(struct sockaddr_in* addr, int port, char *sender_addr) {
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //host to network short encoding
	addr->sin_addr.s_addr = inet_addr(sender_addr);
}

void init_addr_sender(struct sockaddr_in *addr, int port)
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

char *get_file_name(char *path)
{
	return basename(path); //(Do not pass these pointers to free(3).)
}

char *get_archive_name(char *name)
{
	char *archive_file_name;
	archive_file_name = malloc(strlen(name) + 8);
	strncpy(archive_file_name, name, strlen(name));
	strncat(archive_file_name, ".tar.gz", 7);
	return archive_file_name;
}

unsigned int get_file_size(char *name)
{
	unsigned int file_size;
	char *archive_file_name;
	FILE *fp;

	archive_file_name = malloc(strlen(name) + 8); // or 9?
	strncpy(archive_file_name, name, strlen(name));
	strncat(archive_file_name, ".tar.gz", 7); // or 8?

	fp = fopen(archive_file_name, "r");
	if(fp==NULL)
	{

		printf("ERROR: could not open file: %s\n", archive_file_name);
		fclose(fp);
		free(archive_file_name);
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);

	fclose(fp);
	free(archive_file_name);
	return file_size;
}

char* archive(char *filename, char *filepath)
{
  printf("inside..\n");
  char *command;
  command = malloc(20 + strlen(filename) + strlen(filepath));
  sprintf(command, "tar -zcf %s.tar.gz %s", filename, filepath);
  if(system(command) < 0)
  {
  	printf("ERROR: %s could not be archived\n", filename);
  }
  free(command);
  return 0;
}

#endif
