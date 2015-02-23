#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

static unsigned char buff[1024];

static int readline(FILE *f)
{
	unsigned char *p;
	int c;

	p = buff;
	c = fgetc(f);

	if (c == EOF)
		return 1;

	while (c != '\n' && c != EOF)
	{
		*p++ = (unsigned char) c;
		c = fgetc(f);
	}

	*p = 0;

	return 0;
}

int is_valid_symbol(unsigned char* sym)
{
	if (*sym == 0)
		return 0;
	if (strlen(sym) < 2)
		return 0;

	while (*sym != 0)
	{
		if (*sym == '.' || *sym == '/' || *sym == '=')
			return 0;
		sym++;
	}
	return 1;
}

int is_valid_address(unsigned char* addr)
{
	if (*addr == 0)
		return 0;
	while (*addr != 0)
	{
		if (	*addr != '0' &&
			*addr != '1' &&	
			*addr != '2' &&
			*addr != '3' &&
			*addr != '4' &&
			*addr != '5' &&
			*addr != '6' &&
			*addr != '7' &&
			*addr != '8' &&
			*addr != '9' &&
			*addr != 'a' &&
			*addr != 'b' &&
			*addr != 'c' &&
			*addr != 'd' &&
			*addr != 'e' &&
			*addr != 'f' )
			return 0;
		addr++;
	}
	return 1;
}

int main(int argc, char* argv[])
{
	FILE *fin, *fout;
	char symname[100];

	fin = fopen(argv[1], "r");
	fout = fopen(argv[2], "wb");

	if (fin == 0)
	{
		printf("Could not open: %s\n", argv[1]);
		return 0;
	}

	while (!readline(fin))
	{


		buff[34] = 0;
		if (is_valid_address(&buff[18]) && is_valid_symbol(&buff[50]))
		{
			unsigned int addr;
			unsigned short addr_packed;
			sscanf(&buff[18], "%x", &addr); 

			assert(sizeof(unsigned int) == 4);

			strcpy(symname, &buff[50]);
			fwrite(&addr, 4, 1, fout);
			fwrite(symname, 1, 100, fout);

			//printf("%x - %s - %s\n", addr, &buff[18], &buff[50]);

		}
		else if (0)
		{
			printf("NOT:VALID [%s] %s\n", &buff[18], &buff[50]);
		}

	}

	fclose(fout);
	fclose(fin);
	return 0;
}

