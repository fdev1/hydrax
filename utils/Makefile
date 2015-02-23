#
# Project Hydrax Utilities Makefile
#

CC=../tools/bin/i386-elf-gcc
LD=../tools/bin/i386-hydrax-ld

SOURCES=main.o args.o
ECHO_SOURCES=echo.o
PIPETEST_SOURCES=pipetest.o
THREADTEST_SOURCES=threadtest.o

INIT_OBJ=main.o args.o
INIT_EXE=init
ECHO_OBJ=echo.o
ECHO_EXE=echo
PIPE_OBJ=pipetest.o
PIPE_EXE=pipetest
THREAD_OBJ=threadtest.o
THREAD_EXE=threadtest
CHOWN_OBJ=chown.o
CHOWN_EXE=chown

CFLAGS=-g -nostdlib -nostdinc -fno-builtin -I../libs \
	-I../kernel/include -fno-stack-protector
LDFLAGS=-melf_i386

all: init pipe echo thread chown

clean: 
	rm -f *.o 
	rm -f init
	rm -f echo
	rm -f pipetest
	rm -f threadtest
	
init: $(INIT_OBJ)
	@echo -e "  LD\t\t$(INIT_EXE)"
	@$(LD) $(LDFLAGS) -o $(INIT_EXE) $(INIT_OBJ) ../libs/libc.a

echo: $(ECHO_OBJ)
	@echo -e "  LD\t\t$(ECHO_EXE)"
	@$(LD) $(LDFLAGS) -o $(ECHO_EXE) $(ECHO_OBJ) ../libs/libc.a

pipe: $(PIPE_OBJ)
	@echo -e "  LD\t\t$(PIPE_EXE)"
	@$(LD) $(LDFLAGS) -o $(PIPE_EXE) $(PIPE_OBJ) ../libs/libc.a

thread: $(THREAD_OBJ)
	@echo -e "  LD\t\t$(THREAD_EXE)"
	@$(LD) $(LDFLAGS) -o $(THREAD_EXE) $(THREAD_OBJ) ../libs/libc.a

chown: $(CHOWN_OBJ)
	@echo -e "  LD\t\t$(CHOWN_EXE)"
	@$(LD) $(LDFLAGS) -o $(CHOWN_EXE) $(CHOWN_OBJ) ../libs/libc.a 

.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
