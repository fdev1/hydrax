#
# Project Hydrax Utilities Makefile
#

OBJECTS=mkinitrd.o
EXECUTABLE=mkinitrd
CFLAGS=-g -m32 -DNEW

# This program needs to built as a 32-bit executable.
# On Cygwin32 the only way to do this is with the
# cygwin32-gcc-core package.
#
ifeq ($(shell gcc -dumpmachine), x86_64-pc-cygwin)
	CC=i686-pc-cygwin-gcc
	#CC=i686-pc-mingw32-gcc
	#CFLAGS=-g -DNEW
endif

all: mkinitrd

clean: 
	rm -f *.o 
	rm -f mkinitrd
	
mkinitrd: $(OBJECTS)
	@echo -e "  LD\t\t$(EXECUTABLE)"
	@$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) #-lc

.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
