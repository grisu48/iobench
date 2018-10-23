# iobench

Small open/write/close benchmarking utility.

The program iteratively opens a file, writes a number of random bytes and closes the file. It measures the microseconds the system needs for opening, writing and closing the file.

## Build

The program ships with a makefile

    make -j2

## Run

Use the help function of the program:

    iobench2 -h

Example usage: To write `8` blocks in the directory `/SCRATCH` with a block size of `4096` and by having `24` children in parallel:

    iobench2 -b 4096 -f /SCRATCH -c 8 -C 24
