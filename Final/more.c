#include "ucode.c"
#include "helper.c"

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


