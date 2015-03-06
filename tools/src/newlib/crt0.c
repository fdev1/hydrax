
extern int main(int, char**);
extern void exit(int);

int _start(int argc, char **argv)
{
	int ret;
	ret = main(argc, argv);
	exit(ret);
	return ret;
}

