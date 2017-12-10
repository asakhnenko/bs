#ifndef AUFGABE_2_H_
#define AUFGABE_2_H_

#define MTU 1492

#include <stdlib.h>
#include <stdio.h>

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

//--------------------------
/*
 * Checks whether file exists at filename
 * \param *filename path to the file
 */
int file_exists(const char *filepath)
{
  struct stat buffer;
  return (stat(filepath,&buffer) == 0);
}

unsigned int get_pkg_size(unsigned int seq, unsigned int datalen)
{
  unsigned int size;
  if(seq == datalen/MTU)
  {
    size = datalen - MTU*seq;
  }
  else
  {
    size = MTU;
  }
  return size;
}

#endif
