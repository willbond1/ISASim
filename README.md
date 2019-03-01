# ISASim
Semester project for CS 535: Computer Architecture

Cache Specifications:
  L1D:
    32KB Capacity
    8-way
    64 byte line
    4 cycle latency
  L1I:
    32KB Capacity
    8-way
    64 byte line
    4 cycle latency

    Number of lines in cache = cache size / line size (32KB / 64B = 500)
    Number of sets in cache = ceil(number of lines / associativity) (500 / 8 = 63)
    Number of bits required to address set = ceil(log2 number of sets) (log2 63 = 6)
    Number of offsets in line = line size / word size (64B / 32b = 16)
    Number of bits required to address offset = ceil(log2 number of offsets) (log2 16 = 4)
    Number of bits required for tag = word size - (bits in offset + bits in address) (32 - (6 + 4) = 22)

  L2:
    256KB capacity
    64 byte line
    11 cycle latency
  
  L3:
    8MB capacity
    64 byte line
    38 cycle latency

Main Memory:
  1GB capacity
  64 byte line
  100 cycle latency

30 bits in address
6 bits for offset
6 bits for set
18 bits for tag
111111111111111111 111111 111111
