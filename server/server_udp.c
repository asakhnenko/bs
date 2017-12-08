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
/*
 * Checks whether file exists at filename
 * \param *filename path to the file
 */
int file_exists(char *filepath)
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

char* create_command(char *filepath)
{
  char *tmp = "tar cfzv tmp/file.tar.gz ";
  char *command = malloc(strlen(tmp) + strlen(filepath) + 1);
  strcpy(command,tmp);
  strcat(command,filepath);
  return command;
}

void create_archive(char *filepath)
{
	printf("Creating an archive\n");
	char* command = create_command(filepath);
	system(command);
	free(command);
}

char* get_name(char* filepath)
{
  char *name;
  char *search ="/";

  name = strtok(filepath, search);
  while(name != NULL)
  {
    name = strtok(NULL, search);
  }

  return name;
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
  printf("Got it\n");
  if(!file_exists(filepath))
  {
      printf("Please give a valid file path\n");
      return 0;
  }
  printf("Going in\n");
  char *tmp = get_name(filepath);
  //printf(tmp);
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

  unsigned char typID = buff[0];
  if(typID == REQUEST_T)
  {
    //--- Sending
    unsigned char* msg[2048];
    memcpy(msg, &HEADER_T, sizeof(HEADER_T));
    err = sendto(socket_descriptor, msg, sizeof(msg) + 1, 0, (struct sockaddr*) &dest_addr, socklen);
    if(err<0)
    {
        perror("Send error: ");
    }
  }

  system("rm -r tmp");
}
