#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
/*#include <syscall.h>*/
#include <dirent.h>
#include <sys/stat.h>
#include "args.h"

void syscall_test(void);

#define MAX_PATH 255

/*
 * NOTE: This is not being initialized to zero, probably a bug with the
 * elf loader. For now I will just initialize it on main.
 */
static char cwd[MAX_PATH + 1];
static char linebuf[256];
static char *readline(void)
{
	int i = 0;

	while (1)
	{
		char c;
		if (c = getchar())
		{
			switch (c)
			{
				case '\b' :
					if (i)
					{
						i--;
						putchar(c);
					}
					break;

				case '\n' :
				case '\r' :
					putchar('\n');
					linebuf[i] = 0;
					return linebuf;
				default :
					putchar(c);
					linebuf[i++] = c;
					break;
			}
		}
	}
}

/*
 * 
 */
static void parse_command(char *cmd)
{
	int isblank = 1;
	char *pcmd = cmd;
	char buf[50];
	int arg_c;

	while (*pcmd != 0)
	{
		if (*pcmd != ' ')
			isblank=0;
		pcmd++;
	}

	if (isblank)
		return;

	arg_c = argc(cmd);
	pcmd = argv(cmd, 0, buf);

	if (!strcmp(pcmd, "getpid"))
	{
		printf("%i\n", getpid());
	}
	else if (!strcmp(cmd, "exit"))
	{
		exit(0);
	}
	else if (!strcmp(cmd, "kill"))
	{
		kill(1, 7);
	}
	else if (!strcmp(cmd, "clear"))
	{
		/* workaround until we get this on our tty */
		int i;
		for (i = 0; i < 25; i++)
			putchar('\n');
	}
	else if (!strcmp(pcmd, "ls"))
	{
		char *path = cwd;
		struct dirent *dp;	
		
		if (arg_c > 1)
			path = argv(cmd, 1, buf);

		int i;
		/*struct dirent entries[100];*/
		DIR *dirp = opendir(path);
		if (dirp == NULL)
		{
			printf("error: could not open directory.");
			return;
		}
		
		dp = readdir(dirp);
		
		while (dp != NULL)
		{
			char b[100];
			char *p, *bb;
			
			p = path;
			bb = b;
			struct stat st;
			size_t filesz;
			char type[2];
			type[0] = '-';
			type[1] = 0;

			while (*p != 0)
				*bb++ = *p++;

			if (*(p - 1) != '/')
				*bb++ = '/';
			
			p = dp->name;
			while (*p != 0)
				*bb++ = *p++;
			*bb++ = 0;
	
			int ret = stat(b, &st);
			if (ret < 0)
			{	
				printf(" [stat returned %i] ", ret);
			}

			filesz = st.st_size;

			if (S_ISDIR(st.st_mode))
			{
				type[0] = 'd';
			}
			else if (S_ISCHR(st.st_mode))
				type[0] = 'c';

			printf(type);
			printf("-------- ");
			printf("%i:%i\t", st.st_uid, st.st_gid);
			printf("%i\t", filesz);
			printf("%s\n", dp->name);
			
			
			dp = readdir(dirp);
		}
		
		putchar('\n');
		closedir(dirp);
		
#if 0
		int ent_c = readdir(rootfd, entries, 100);
		printf("Found %i entries.\n", ent_c);

		for (i = 0; i < ent_c; i++)
		{
			char b[100];
			char *p, *bb;
			
			p = path;
			bb = b;;
			struct stat st;
			size_t filesz;
			char type[2];
			type[0] = '-';
			type[1] = 0;

			while (*p != 0)
				*bb++ = *p++;

			if (*(p - 1) != '/')
				*bb++ = '/';
			
			p = entries[i].name;
			while (*p != 0)
				*bb++ = *p++;
			*bb++ = 0;
	
			int ret = stat(b, &st);
			if (ret < 0)
			{	
				printf(" [stat returned %i] ", ret);
			}

			filesz = st.st_size;

			if (S_ISDIR(st.st_mode))
			{
				type[0] = 'd';
			}
			else if (S_ISCHR(st.st_mode))
				type[0] = 'c';

			printf(type);
			printf("-------- ");
			printf("%i:%i\t", st.st_uid, st.st_gid);
			printf("%i\t", filesz);
			printf("%s\n", entries[i].name);

		}
		putchar('\n');
		close(rootfd);
#endif
	}
	else if (!strcmp(pcmd, "time"))
	{
		time_t t;
		time(&t);
		printf("time() returned %i.\n", t);
	}
	else if (!strcmp(pcmd, "pwd"))
	{
		char buf[MAX_PATH + 1];
		getcwd(buf, MAX_PATH + 1);
		printf("%s\n", buf);

	}
	else if (!strcmp(pcmd, "cd"))
	{
		char *path;
		if (arg_c < 2)
		{
			printf("Invalid arguments!\n");
			return;
		}
		path = argv(cmd, 1, buf);
		chdir(path);
	}
	else if (!strcmp(pcmd, "cat"))
	{
		int fd;
		char *path;
		char filepath[MAX_PATH + 1];
		char filebuf[1025];
		int len;
		int bytes_read = 0;

		if (arg_c < 1)
		{
			printf("Invalid argument.\n");
			return;
		}
		path = argv(cmd, 1, buf);
		fd = open(path, 0);
		if (fd < 0)
		{
			printf("Error opening file: %s\n", path);
			return;
		}

		while (len = read(fd, filebuf, 1024)) 
		{
			filebuf[len] = 0;
			printf("%s", filebuf);
			if (len < 1024)
				break;
		}
		putchar('\n');
		close(fd);

	}
	else if (!strcmp(cmd, "reboot"))
	{
		reboot();
	}
	else if (!strcmp(pcmd, "trace"))
	{
		syscall_test();
	}
	else if (!strcmp(pcmd, "export"))
	{
		char buf[255]; 
		char *name;
		char *value;
		char *arg_v;
		
		if (arg_c != 2)
		{
			printf("Invalid arguments argc=%i\n", arg_c);
			return;
		}
		
		arg_v = argv(cmd, 1, buf);
		while (*argv != NULL && *arg_v != '=')
			*name++ = *arg_v++;
		if (*arg_v != '=')
		{
			printf("Invalid arguments.\n");
			return;
		}
		value = ++arg_v;
		*name = 0;
		if (setenv(buf, value, 1) == 0)
			return;
	}
	else
	{
		int fd;
		struct stat st;
		char file[255];
		
		strcpy(file, pcmd);

		fd = stat(file, &st);
		if (fd < 0)
		{
			printf("command %s not found\n", pcmd);
		}
		else
		{
			pid_t pid;
			pid = fork();
			
			if (pid == 0)
			{
				int i;
				char arg_v[20][100];
				char *args[20];
				
				for (i = 0; i < arg_c - 1; i++)
				{
					args[i] = argv(cmd, i + 1, &arg_v[i][0]);
					printf("arg %i = %s\n", i, args[i]);
				}
				args[i] = NULL;

				execve(file, args, args);
				printf("execve %s failed!\n", pcmd);
			}
			else
			{
				int exit_code;
				char buffy[10];
				waitpid(pid, &exit_code, 0);
				/* setenv("!", itoa(exit_code, buffy, 10), 1); */
			}
		}
	}
}

/*
 * 
 */
int main(int argc, char **argv)
{
	printf("\nHydrax Shell (pid=%i argc=%i) v0.1\n", getpid(), argc);
	
	if (0 && argc)
	{
		int i;
		for (i = 0; i < argc; i++)
			printf("arg: %x %x %x %s\n", &argv, argv, argv[i], argv[i]);
	}

	while (1) 
	{
		getcwd(cwd, MAX_PATH + 1);
		printf("%s $ ", cwd);
		parse_command(readline());
	}
		
	return 0;
}

