#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "mod_exp.h"
#include "brpa3.h"

unsigned short order = 59;
unsigned short generator = 2;
unsigned short secret = 5;
unsigned short openkey = 32;
unsigned short openkey_sender = 16;

static unsigned long encrypt(unsigned int toEncrypt)
{
	unsigned long encrypted;
	encrypted = mod_exp(openkey_sender, secret, order);
	encrypted = encrypted * toEncrypt % order;
	return encrypted;

}

static unsigned int output_size(unsigned long number)
{
	unsigned int size;

	size = 0;
	while(number > 0)
	{
		size = size + 1;
		number =  number / 10;
	}
	return size;
}

static char* number2output(unsigned long number)
{
	if(number < 0)
	{
		printf("Wrong number!");
		return 0;
	}
	unsigned int size = output_size(number);;
	
	char* output = (char*)malloc(size);
	int i;
	for(i = size-1; i >= 0; i--)
	{
		output[i] = (number % 10) + 48;
		number = number / 10;
	}
	return output;
}

int main(int argc, char **argv)
{

	char* encryptedString;
	char* decryptedString;
	char endLine[1];
	
	unsigned long nrcnt, encrypted;
	unsigned int buffer_size;
	int fd;

	// opening device file
	fd = open("/dev/brpa3_961737_959063", O_RDWR);
	if (fd < 0) {
		printf("LKM brpa3_961737_959063 does not exist in kernel!\n");
		return -1;
	}
	
	secret = 9;
	if (ioctl(fd, BRPA3_SET_SECRET, &secret)) {
		perror("Setting key failed");
		exit(1);
	}
	
	printf("Secret set to: %d\n", secret);
	
	if (ioctl(fd, BRPA3_GET_OPENKEY, &openkey) < 0) {
		perror("Getting openkey failed");
		exit(1);
	}

	printf("New openkey: %d\n\n", openkey);
	

	// Iterate..
	for (nrcnt = 1; nrcnt < 59; ++nrcnt) {

		// encrypting the original number
		encrypted = encrypt(nrcnt);
		buffer_size = output_size(encrypted);
		encryptedString = number2output(encrypted);
		printf("encrypting number %lu to %lu..\n", nrcnt, encrypted);
		printf("sending %lu to LKM..\n", encrypted);


		// writing the encrypted number to device file
		if(write(fd, encryptedString, buffer_size) <0 )
		{
			free(encryptedString);
			printf("ooops, something went wrong while writing!\n");
			return -1;
		}
		// reading the decrypted number from the device file
		decryptedString = malloc(20);
		if(read(fd, decryptedString, 20) < 0)
		{
			free(encryptedString);
			free(decryptedString);
			printf("ooops, something went wrong while reading!\n");
			return -1;
		}
		if(read(fd, endLine, 0) < 0)
		{
			free(encryptedString);
			free(decryptedString);
			printf("ooops, something went wrong while reading!\n");
			return -1;
		}

		printf("received decrypted number: %s \n\n", decryptedString);

		free(encryptedString);
		free(decryptedString);
	}

	return 0;
}