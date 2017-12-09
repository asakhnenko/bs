#include<arpa/inet.h>
#include<libgen.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>

#include"../Aufgabe2.h"

#define MTU 1492
/*
 * Checks whether file exists at filename
 * \param *filename path to the file
 */
int file_exists(const char *filepath)
{
  struct stat buffer;
  return (stat(filepath,&buffer) == 0);
}

void init_addr(struct sockaddr_in* addr, int port)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //host to network short encoding
	addr->sin_addr.s_addr = inet_addr("127.0.0.1");
}

char* get_name(char* filepath)
{
  char* name = basename(filepath);
  char* extension = ".tar.gz";
  char* ret = (char*)malloc(sizeof(char) * (strlen(name) + strlen(extension) + 2));
  strcpy(ret, name);
  strcat(ret, extension);
  return ret;
}

char* create_command(char *filepath)
{
  char *tmp = "tar -cf tmp/";
  char *name = get_name(filepath);
  char *command = (char*)malloc(sizeof(char)*(strlen(tmp) + strlen(filepath) +strlen(name) + 1));

  strcpy(command, tmp);
  strcat(command, name);
  strcat(command, " ");
  strcat(command, filepath);

  //free(name);
  return command;
}

void create_archive(char *filepath)
{
	printf("Creating an archive\n");
	char* command = create_command(filepath);
	system(command);
	free(command);
}

unsigned int get_file_size(char *name)
{
  char path[strlen(name)+5];
  strcpy(path, "./tmp/");
  strcat(path, name);

  FILE *file = fopen(path,"rb");
  if(!file)
  {
    printf("Could not find archived file\n");
  }

  fseek(file, 0, SEEK_END);
  unsigned int size = ftell(file);
  fclose(file);

  return size;
}

int main(int argc, char *argv[])
{
  //----------------------------------
  //--- Check user input
  if(argc < 3)
  {
      printf("Please provide port number and a file path\n");
      return 0;
  }

  int port;
  port = atoi(argv[1]);
  if(port == 0)
  {
      printf("Please provide a number for port\n");
      return 0;
  }

  char *filepath;
  filepath = argv[2];
  if(!file_exists(filepath))
  {
      printf("Please give a valid file path\n");
      return 0;
  }

  //----------------------------------
  int socket_descriptor, dest_socket_descriptor, err, socklen;
  struct sockaddr_in addr, dest_addr;

  //--- Initiate a socket
  socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if(socket_descriptor < 0)
  {
      perror("Socket was not created: ");
  }
  printf("Socket is initialized\n");

  //--- Set time limit
  struct timeval timeout;
  timeout.tv_sec = 10;
  err = setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if(err<0)
  {
     perror("Couldn't set timeout option : ");
  }

	//----------------------------------
  //--- Fill in sockaddr_in
  init_addr(&addr, port);

  //--- Archive the file
  system("mkdir tmp");

  // Create the tar file
  create_archive(filepath);

  //--- Binding
  socklen = sizeof(struct sockaddr_in);
  err = bind(socket_descriptor, (struct sockaddr *) &addr, socklen);
  if(err<0)
  {
      perror("Binding Error: ");
  }

  //--- Receiving request
  unsigned char buff[2048];
  printf("Waiting for connection...\n");
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
  // Answering the request
  unsigned char typID = buff[0];
  if(typID == REQUEST_T)
  {
    printf("Request received\nCreating header:\n");

    //--- Sending Header
    // Creating message
    char *name = get_name(filepath);
    printf("   File name: %s\n", name);
    unsigned short namelen = strlen(name);
    printf("   File name length: %d\n", namelen);
    unsigned int datalen = get_file_size(name);
    printf("   File size: %d\n", datalen);
    unsigned char* msg = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned short) + namelen + sizeof(unsigned int));

    memcpy(msg, &HEADER_T, sizeof(unsigned char));
    memcpy(msg + sizeof(unsigned char), &namelen, sizeof(unsigned short));
    //strlen counts size till null-terminal
    memcpy(msg + sizeof(unsigned char) + sizeof(unsigned short), name, namelen);
    memcpy(msg + sizeof(unsigned char) + sizeof(unsigned short) + namelen, &datalen, sizeof(unsigned int));

    err = sendto(socket_descriptor, msg, sizeof(msg) + 1, 0, (struct sockaddr*) &dest_addr, socklen);
    if(err<0)
    {
        perror("Send error: ");
    }
    printf("   Message size in bytes: %d\n", err);
    free(msg);

    //----------------------------------
    //--- Sending Files
    printf("Sending the files\n");

    unsigned char file_buff[MTU];
    FILE *file = fopen("./tmp/test3.tar.gz","rb");
    int i = 0;
    int c;
    unsigned int seq;
    do
    {
      c = fgetc(file);
      file_buff[i % MTU] = c;
      if(i++ % MTU == 0)
      {
        seq = i/MTU;
        printf("Sending %d-st package\n", seq);
        unsigned char* file_pkg = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int) + MTU);
        memcpy(file_pkg, &DATA_T, sizeof(unsigned char));
        memcpy(file_pkg + sizeof(unsigned char), &seq, sizeof(unsigned int));
        memcpy(file_pkg + sizeof(unsigned char) + sizeof(unsigned int), file_buff, MTU);
        free(file_pkg);
      }
    }
    while(c != EOF);


  }

  system("rm -r tmp");
}
