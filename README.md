# iobench

[![Build Status](https://travis-ci.org/grisu48/iobench.svg?branch=master)](https://travis-ci.org/grisu48/iobench)

`iobench2` is a small open/write/close benchmarking utility for parallel writes on an arbitrary filesystem.

The program forks `N` children, that each open a file, write `n` times a random block of bytes, close the file and report the stats to the parent process.
The parent process collects the stats from all of the children and reports it to the user.

I wrote this program to benchmark real world applications on a rather large filesystem, where multiple users and program access different filesystems randomly.


## Build

The program ships with a makefile

    make -j2

## Run

Use the help function of the program:

    iobench2 -h

Example usage: To write `8` blocks in the directory `/SCRATCH` with a block size of `4096` and by having `4` children in parallel:

    iobench2 -b 4096 -f /SCRATCH -c 8 -C 4
    
                      Init     Open     Write    Close   
      Child   1:      151       61      355       16 µs
      Child   2:      138       42      172       37 µs
      Child   3:      129       39      226       43 µs
      Child   4:      137       48      139       15 µs
    
      Overall         Init     Open     Write    Close
      Sum      :      555      190      892      111 µs
      Min      :      129       39      139       15 µs
      Max      :      151       61      355       43 µs
      Average  :      138       47      223       27 µs
    Total: 131072 bytes in 162 ms (788.56 kB/s)

