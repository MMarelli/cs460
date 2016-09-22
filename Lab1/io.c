#define BASE 10
#define HEX 16

char *table = "0123456789ABCDEF";

/*recursive helper for printu*/
void rpu(unsigned long x)
{
	char c;
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putc(c);
	}
}


/* Printing Recursive Helper Functions
// recursive helper for printu*/
void rpd(int x)
{
	char c;
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putc(c);
	}
}

/* recursive helper for printu */
void rpx(unsigned long x)
{
	char c;
	if (x)
	{
		c = table[x % HEX];
		rpx(x / HEX);			
		putc(c);
	}
}

/*/ Printing Helper Functions
// print an integer*/
void printd(int x)
{
	if (x == 0)
		putc('0');
	else if (x < 0)
	{
		putc('-');
		rpd(-x);
	}
	else
		rpd(x);
	putc(' ');
}

/*/ print a hex value*/
void printx(unsigned long x)
{
	if (x == 0)
		putc('0');
	else
		rpx(x);
	putc(' ');
}

/*/ print a character*/
void printc(char c)
{
	putc(c);
}

/*/ print a string*/
void prints(char *s)
{
	int i = 0;
	for (i = 0; s[i] != '\0'; i++)
		putc(s[i]);
	putc(' ');
}
/*/ prints an unsigned long (non-negative number)*/
void printu(unsigned long x)
{
	if (x == 0)
		putc('0');
	else
		rpu(x);
	putc(' ');
}

void gets(char *s)
{
	int i = 0;
	char input = '\0';

	while(input = getc() != '\n');
	{
		s[i++] = input;
	}
}

int myprintf(char *fmt, ...)
{
	int i = 0;
	int *ip = &fmt + 1;

	if (strcmp(fmt, "") != 0)
	{
		for (i = 0; fmt[i] != '\0'; i++)
		{
			if (fmt[i] == '%')
			{
				i++;
				if (fmt[i] == 'c')
					printc(*ip);
				else if (fmt[i] == 'd')
					printd(*ip);
				else if (fmt[i] == 'x')
					printx(*ip);
				else if (fmt[i] == 's')
					prints(*ip);
				else if (fmt[i] == '%')
					putc('%');
				else
					return 0;
				ip++;
			}
			else if (fmt[i] == '/')
			{
				i++;
				if (fmt[i] == 'n')
				{
					putc('\n');
					putc('\r');
				}
				i++;
			}
			else
				putc(fmt[i]);
		}
	}
	return 1;
}