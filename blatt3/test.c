#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

#include "mod_exp.h"
#include "brpa3.h"

unsigned short order = 59;
unsigned short generator = 2;
unsigned short secret = 5;
unsigned short openkey = 32;
unsigned short openkey_sender = 16;

static long input2number(char *string, size_t size)
{
	long result = 0;
	int i;
	unsigned int decimal = 1;
	// TODO
	if(size > 9)
	{
		printf("Wrong input!");
		return -1;
	}

	for(i = size - 1; i >= 0; i--)
	{
		if(string[i] < 48 || string[i] > 57)
		{
			printf("Wrong input!");
			return -1;
		}
		result = result + (int)(string[i] - 48) * decimal;
		decimal = decimal * 10;
	}
	return result;
}

static unsigned long decrypt(unsigned int decrypted)
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

int main(int argc, char *argv[])
{
	int fd = open("/dev/reverse", O_RDWR);
	if(fd < 0)
	{
		perror("Failed to open a device");
	}
	unsigned short secret = 1;
	ioctl(fd, BRPA3_SET_SECRET, &secret);
	write(fd, argv[1], strlen(argv[1]));
	long test = input2number(argv[1], strlen(argv[1]));
	unsigned long test3 = decrypt(test);
	char* test2 = number2output(test3);
	printf("Encrypted in test: %s\n", test2);
	char try[strlen(argv[1])];
	read(fd, try, strlen(argv[1]));
	printf("Read: %s \n", try);

	close(fd);

	return 0;
}
