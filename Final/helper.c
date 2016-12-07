
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
