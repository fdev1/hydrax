#
# Project Hydrax Utilities Makefile
#

OBJECTS=main.o
EXECUTABLE=symclean
CFLAGS=-g -DNEW

all: symclean

clean: 
	rm -f *.o 
	rm -f symclean
	
symclean: $(OBJECTS)
	@echo -e "  LD\t\t$(EXECUTABLE)"
	@$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) -lc

.c.o:
	@echo -e "  CC\t\t"$<
	@$(CC) $(CFLAGS) -c $< -o $@
