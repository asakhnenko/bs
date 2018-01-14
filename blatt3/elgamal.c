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
	for (nrcnt = 10; nrcnt < 59; ++nrcnt) {
		printf("enter loop\n");

		// encrypting the original number
		encrypted = encrypt(nrcnt);
		printf("CP1\n");
		// writing the encrypted number to device file
		write(fd, &encrypted, sizeof(encrypted));
		printf("CP2 %d\n", encrypted);
		// reading the decrypted number from the device file
		read(fd, &decrypted, sizeof(decrypted));
		printf("CP3\n");

		printf("%d\n", decrypted);
		
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

/**
 * @brief Prints usage information
 * @details Prints information to run the load script and some 
 * additional message that can be passed as an argument.
 * 
 * @param message 	The message that precedes the load script information. If NULL 
 * 					or an empty string is passed, it will be skipped
 */

 /**
void print_module_usage(char *message)
{
	char *usage = "In order to test the module, please run the shell script 'load_lkm.sh' before executing this file.\n";

	if (message && strlen(message) > 0) {
		printf("%s\n%s", message, usage);
	} else {
		printf(usage);
	}
}
*/
/**
 * @brief Generates a public key
 * @details Computes a public key from the private key and the cyclic group (p, g) using the Elgamal Method
 * 
 * @param generator 	The generator (g) as part of the cyclic group
 * @param secret 		The private key of this instance
 * @param order 		The order (p) as part of the cyclic group
 * 
 * @return A public key that can be shared with the communication partners
 */
 /**
param_t generate_openkey(param_t generator, param_t secret, param_t order)
{
	return (param_t) mod_exp(generator, secret, order);
}
*/
/**
 * @brief Encrypts a number
 * @details Encrypts a positive number using the private key, the public key 
 * of the receiver and the order of the cyclic group. This encryption uses 
 * the Elgamal Encryption.
 * 
 * @param openkey_receiver 	The public key of the communication partner
 * @param secret 			The private key of this instance
 * @param nrcnt 			The message (number) that needs to be encrypted
 * @param order 			The order (part of the cyclic group)
 * 
 * @return The encrypted message (number)
 */
 /**
number_t encrypt(param_t openkey_receiver, param_t secret, number_t nrcnt, param_t order)
{
	number_t encrypted = mod_exp(openkey_receiver, secret, order);
	encrypted *= nrcnt % order;

	return encrypted;
}
*/