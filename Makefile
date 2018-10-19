

# Default compiler and compiler flags
CC=gcc

# Default flags for all compilers
O_FLAGS=-O3 -Wall -Werror -Wextra -pedantic
# Debugging flags
#O_FLAGS=-Og -g2 -Wall -Werror -Wextra -pedantic
CC_FLAGS=$(O_FLAGS) -std=c99


# Default generic instructions
default:	all
all:	iobench
clean:
	
iobench: iobench.c
	$(CC) $(CC_FLAGS) -o $@ $<
	
install: iobench
	install iobench /usr/local/bin/
