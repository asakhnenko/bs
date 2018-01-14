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



int main(int argc, char **argv)
{
	unsigned long nrcnt, encrypted, decrypted;
	int fd;

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
	for (nrcnt = 1; nrcnt < 59; ++nrcnt) {
		printf("enter loop\n");

		// encrypting the original number
		encrypted = encrypt(nrcnt);
		// writing the encrypted number to device file
		write(fd, &encrypted, sizeof(encrypted));
		// reading the decrypted number from the device file
		read(fd, &decrypted, sizeof(decrypted));

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