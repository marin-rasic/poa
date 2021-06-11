# poa

Poa is a c++ library for alignment of multiple partial order graphs.

# Install

To build poa run the following commands:

```
$ git clone https://github.com/marin-rasic/poa.git && cd poa && git submodule update --init --recursive 
  && mkdir build && cd build && cmake .. && make
```

The program will be installed inside /build/bin/ directory.

# Usage

```
poa <sequence_file> [options]
```
The sequence file should be in fasta format.
The avaliable options are:
```
-a, --alignment <int>
    default: 0
    alignment alogrithm:
         0: Global alignment
         1: Local alignment
         2: Semiglobal alignment
-m, --match <int>
    default: 5
    score for matching bases
-n, --mismatch <int>
    defualt: -4
    score for mismatching bases
-g, --gap <int>
    default: -8
    linear gap penalty
-t, --threads <int>
    default: 1
    number of threads
-h, --help
    Print usage information
```

