/***************************
Programer: Michael Marelli
Class: CS460
Project: Final
Date: 12/9/2016
File: more.c
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
	int in = 0, stdout = 1, ttyin, ttyout, lines, count;
	char buf[1024], c, *tty;

	gettty(tty);
	ttyin = open(tty, O_RDONLY);
	ttyout = open(tty, O_WRONLY);
	if(argc == 2)
	{
		in = open(argv[1], O_RDONLY);
	}

	for(lines = 20; lines > 0; lines--)
	{
		count = readLine(in, buf);
		if(count == 0)
		{
			exit(1);
		}
		write(stdout, buf, count);
		write(ttyout, "\r", 1);
	}
	while(1)
	{
		read(ttyin, &c, 1);
		if(c == '\r')
		{
			lines = 1;
		}
		else if (c == ' ')
		{
			lines = 20;
		}
		else if(c == 'q')
		{
			exit(1);
		}
		else
		{
			continue;
		}
		for(;lines > 0; lines--)
		{
			count = readLine(in, buf);
			if(count == 0)
			{
				exit(1);
			}
			write(stdout, buf, count);
			write(ttyout, "\r", 1);
		}
	}
}


