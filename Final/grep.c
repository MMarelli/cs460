#include "ucode.c"

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

	while(read(in, &c, 1))
	{		
		if(c == '\n' || c == '\r')
		{
			buf[i++] = '\n';
			if(strstr(buf, argv[1]))
			{
				write(stdout, buf, i);
				write(ttyout, "\r", 1);
			}
			i = 0;
			bzero(buf, 1024);
		}
		else
		{
			buf[i++] = c;
		}
	}

	exit(1);
}