#
# Project Hydrax Utilities Makefile
#

OBJECTS=mkinitrd.o
EXECUTABLE=mkinitrd
CFLAGS=-g -m32 -DNEW

all: mkinitrd

clean: 
	rm -f *.o 
	rm -f mkinitrd
	
mkinitrd: $(OBJECTS)
	@echo -e "  LD\t\t$(EXECUTABLE)"
	@gcc -m32 -o $(EXECUTABLE) $(OBJECTS) -lc

.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
