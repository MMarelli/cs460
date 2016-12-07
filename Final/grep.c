/***************************
Programer: Michael Marelli
Class: CS460
Project: Final
Date: 12/9/2016
File: grep.c
****************************/
#include "ucode.c"

int readLine(int in, char*buf)
{
	int ok, i = 0;
	char c;
	while(1)
	{
		ok = read(in, &c, 1);
		if(!ok)
		{
			return 0;
		}
		if(c == '\n' || c == '\r')
		{
			buf[i++] = '\n';
			return i;
		}
		else
		{
			buf[i++] = c;
		}
	}
}

main(int argc, char** argv)
{
	int in = 0, stdout = 1, i = 0, ttyout;
	char buf[1024], c, *tty; //c is used as input char
	gettty(tty);
	ttyout = open(tty, O_WRONLY); //write to stdout

	if(argc == 3)
	{
		in = open(argv[2], O_RDONLY);
	}

	while(readLine(in, buf))
	{
		if(strstr(buf, argv[1]))
		{
			write(stdout, buf, strlen(buf));
			write(ttyout, "\r", 1);
		}
		i = 0;
		bzero(buf, 1024);
	}

	exit(1);
}
