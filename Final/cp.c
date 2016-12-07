/***************************
Programer: Michael Marelli
Class: CS460
Project: Final
Date: 12/9/2016
File: cp.c
****************************/

#include "ucode.c"

main(int argc, char** argv)
{
	int source, dest, bread;
	char buf[1024];

	if(argc != 3)
	{
		printf("Usage: cp <source> <dest>\n");
		exit(1);
	}
	source = open(argv[1], O_RDONLY);
	dest = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT);
	bzero(buf, 1024);

	while(bread = read(source, buf, 1024))
	{
		write(dest, buf, bread);
	}

	exit(1);
}