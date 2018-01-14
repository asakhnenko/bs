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

unsigned short order = 59;
unsigned short generator = 2;
unsigned short secret = 5;
unsigned short openkey = 32;
unsigned short openkey_sender = 16;

static unsigned long encrypt(unsigned int decrypted)
{
	unsigned short power = secret * (order - 2);
	unsigned long b = mod_exp(openkey_sender, power, order);
	return mod_exp((b * decrypted), 1, order);
}

static char* number2output(unsigned long number)
{
	if(number < 0)
	{
		printf("Wrong number!");
		return 0;
	}
	unsigned long tmp = number;
	unsigned int size = 0;
	while(tmp > 0)
	{
		size = size + 1;
		tmp =  tmp / 10;
	}
	char* output = (char*)malloc(size);
	int i;
	for(i = size-1; i >= 0; i--)
	{
		output[i] = (number % 10) + 48;
		number = number / 10;
	}
	return output;
}

static long input2number(char *string, size_t size)
{
	long result = 0;
	int i;
	unsigned int decimal = 1;

	for(i = size - 1; i >= 0; i--)
	{
		result = result + (int)(string[i] - 48) * decimal;
		decimal = decimal * 10;
	}
	return result;
}

int main(int argc, char **argv)
{
	unsigned long nrcnt, encrypted, decrypted;
	char* encryptedString;
	char decryptedString[20];
	int fd, nrBytes;

	size_t buffer_size;

	/**
	secret = 4;
	openkey_sender = 16
	*/

	// opening device file
	fd = open("/dev/reverse", O_RDWR);
	if (fd < 0) {
		return 0;
	}

	// Iterate..
	for (nrcnt = 13; nrcnt < 59; ++nrcnt) {
		printf("enter loop (%d)\n", nrcnt);

		// encrypting the original number
		encrypted = encrypt(nrcnt);
		encryptedString = number2output(encrypted);

		printf("encryptedString %s\n", encryptedString);

		// writing the encrypted number to device file
		write(fd, encryptedString, 2);
		// reading the decrypted number from the device file
		nrBytes = read(fd, decryptedString, 20);

		printf("debug: %s\n", decryptedString);
		printf("bytes: %d\n", nrBytes);

		decrypted = input2number(decryptedString, 3);
		printf("cp2\n");
		printf("decrypted: %d\n", decrypted);
		
		sleep(1);
	}



	/*
	// Set new private key in the LKM
	secret_receiver = 9;
	if (ioctl(fd, BRPA3_SET_SECRET, &secret_receiver)) {
		perror("ioctl call to set secret failed");
		exit(1);
	}

	// Get the new openkey of the LKM
	if (ioctl(fd, BRPA3_GET_OPENKEY, &openkey_receiver) < 0) {
		perror("ioctl call to get openkey failed");
		exit(1);
	}
	*/

	return 0;
}