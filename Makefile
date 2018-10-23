

# Default compiler and compiler flags
CC=gcc

# Default flags for all compilers
O_FLAGS=-O3 -Wall -Wextra
# Debugging flags
#O_FLAGS=-Og -g2 -Wall -Werror -Wextra -pedantic
CC_FLAGS=$(O_FLAGS) -std=c99


# Default generic instructions
default:	all
all:	iobench2
clean:
	
iobench2: iobench2.c
	$(CC) $(CC_FLAGS) -o $@ $< -D_POSIX_C_SOURCE -D_XOPEN_SOURCE
	
install: iobench
	install iobench2 /usr/local/bin/
