# iobench

Small open/write/close benchmarking utility.

The program iteratively opens a file, writes a number of random bytes and closes the file. It measures the microseconds the system needs for opening, writing and closing the file.

## Build

    compile:  gcc -Wall -Wextra -pedantic -std=c99 -o iobench iobench.c

## Run

    ./iobench FILE [BS] [ITERATIONS]

`FILE` is just a simple test file, `BS` defined the block size in bytes, and `ITERATIONS` the number of iterations.
