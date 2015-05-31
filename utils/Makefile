#
# Project Hydrax Utilities Makefile
#

CC=../tools/bin/i386-hydrax-gcc
LD=../tools/bin/i386-hydrax-ld

ECHOFLAGS=-e

ifneq ($(shell echo $(ECHOFLAGS) "Hello"),Hello)
        ECHOFLAGS=
endif


LIBC=../tools/i386-hydrax/lib/libc.a 
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

CFLAGS=-g -fno-stack-protector
LDFLAGS=-melf_i386

all: init pipe echo chown

clean: 
	rm -f *.o 
	rm -f init
	rm -f echo
	rm -f pipetest
	rm -f threadtest
	
init: $(INIT_OBJ)
	@echo $(ECHOFLAGS) "  LD\t\t$(INIT_EXE)"
	@$(LD) $(LDFLAGS) -o $(INIT_EXE) $(INIT_OBJ) -L../tools/i386-hydrax/lib/ -lc

echo: $(ECHO_OBJ)
	@echo $(ECHOFLAGS) "  LD\t\t$(ECHO_EXE)"
	@$(LD) $(LDFLAGS) -o $(ECHO_EXE) $(ECHO_OBJ) -L../tools/i386-hydrax/lib/ -lc

pipe: $(PIPE_OBJ)
	@echo $(ECHOFLAGS) "  LD\t\t$(PIPE_EXE)"
	@$(LD) $(LDFLAGS) -o $(PIPE_EXE) $(PIPE_OBJ) -L../tools/i386-hydrax/lib/ -lc

thread: $(THREAD_OBJ)
	@echo $(ECHOFLAGS) "  LD\t\t$(THREAD_EXE)"
	@$(LD) $(LDFLAGS) -o $(THREAD_EXE) $(THREAD_OBJ) -L../tools/i386-hydrax/lib/ -lc

chown: $(CHOWN_OBJ)
	@echo $(ECHOFLAGS) "  LD\t\t$(CHOWN_EXE)"
	@$(LD) $(LDFLAGS) -o $(CHOWN_EXE) $(CHOWN_OBJ) -L../tools/i386-hydrax/lib/ -lc

.c.o:
	@echo $(ECHOFLAGS) "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
