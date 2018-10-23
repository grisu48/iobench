# iobench

Small open/write/close benchmarking utility.

The program iteratively opens a file, writes a number of random bytes and closes the file. It measures the microseconds the system needs for opening, writing and closing the file.

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

## iobench

`iobench` is a legacy tool and should not be used anymore.