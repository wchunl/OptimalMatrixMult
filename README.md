# Optimized Matrix Multiplication

A program that optimizes matrix multiplication by exploiting spatial and temporal locality within memory. 

## Methods

The different methods of matrix multiplication is compared and its execution time and speedup is measured below. Note that these tests are ran with a 2.20 GHz CPU on Linux. First, an unoptimized kernel is measured. Second, transposition of the matrices on the unoptimized kernel is analyzed. Finally, tiling optimization is implemented, in which the matrices are partitioned into smaller, sub-matrcices that are multiplied one by one.


## Execution time and Speedup of various methods

Speedup is measured against the unoptimized kernel.

| Method | Execution Time | Speedup |
| --- | --- | --- |
| **Unoptimized Kernel** | 3.88 seconds |  |
| **Transpose Optimization**| 1.93 secconds | 2.01x speedup |
| **Tiling with tile size 1** | 2.568 seconds | 1.51x speedup |
| **Tiling with tile size 2** | 1.728 seconds | 2.24x speedup |
| **Tiling with tile size 4** | 1.456 seconds | 2.66x speedup |
| **Tiling with tile size 8** | 1.434 seconds | 2.70x speedup |
| **Tiling with tile size 16** | 1.576 seconds | 2.46x speedup |
| **Tiling with tile size 32** | 1.846 seconds | 2.10x speedup |
| **Tiling with tile size 64** | 2.064 seconds | 1.87x speedup |
| **Tiling with tile size 128** | 2.730 seconds | 1.42x speedup |
| **Tiling with tile size 256** | 4.702 seconds | 0.82x speedup |
| **Tiling with tile size 512** | 4.466 seconds | 0.86x speedup |
| **Tiling with tile size 1024** | 4.240 seconds | 0.91x speedup |

It is apparent that the tiling method with a matrix size of 8 yields the highest speedup (2.7x) compared with the unoptimized matrix multiplication, saving nearly 2.5 seconds of execution time. It is interesting to note, however, with tile sizes above 256, there is a trend in execution time that is taking longer than the unoptimized matrix multiplication.

## Compilation and Execution

To compile the program:
>```gcc -g -o matmul -Wall -O3 matmul.c -lm```

Usage:
>```./matmul [runTimes] [power]```

where `runTimes` = number of times to run the tests
and `power` = the number of tiles in powers of 2


for example, to run it 5 times with 1-1024 tiles:
>```./matmul 5 10```

or to run it 1 time with 1-64 tiles:
>```./matmul 1 6```

At the end of running, a summary with average times from each test is printed to stdout.