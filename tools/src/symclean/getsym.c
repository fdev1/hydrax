#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct 
{
	unsigned int address;
	char name[100];
}
symbol_t;

symbol_t symbols[1000];


static int loadsyms(char* file)
{

	FILE* f = fopen(file, "rb");

	memset(symbols, 0, sizeof(symbol_t) * 1000);

	if (f)
	{
		fread(symbols, sizeof(symbol_t), 1000, f);
	}
	fclose(f);
}

int main(int argc, char* argv[])
{
	unsigned int i;
	unsigned int addr;
	symbol_t cur_sym = { 0,0};

	loadsyms(argv[1]);
	sscanf(argv[2], "%x", &addr);

	printf("Looking for 0x%x\n", addr);
	
	for (i = 0; i < 1000; i++)
	{
		if (symbols[i].address <= addr)
		{
			if (symbols[i].address > cur_sym.address)
				cur_sym = symbols[i];
		}
		//printf("0x%x - %s\n", symbols[i].address, symbols[i].name);
	}

	printf("%s\n", cur_sym.name);

}

