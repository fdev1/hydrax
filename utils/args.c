#include <arch/stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int argc(const char *line)
{
	int i;
	const char *l;
	i = 1;
	l = line;
	while (*l != 0)
	{
		if (*l == ' ')
			i++;
		l++;
	}
	return i;
}

char *argv(char *line, int index, char *buf)
{
	int i;
	char *b, *bb, *bbb;
	bb = b = line;
	i = 0;

	while (1)
	{
		while (*b != 0 && *b != ' ')
			b++;

		if (i >= index || *b == 0)
			break;
		
		bb = ++b;
		i++;
	}

	bbb = buf;
	while (bb < b)
		*buf++ = *bb++;
	*buf++ = 0;
	return bbb;
			
}


