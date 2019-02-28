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

    Memory size = 1GiB = 2^30
    Block size = 16 Bytes = 2^6

    Number of sets in cache = Cache size/(Set size * Block size) = 32KB/(8 blocks * 64B) = 2^6

    Number of bits in Tag = Total bits - Index bits - Offset bits = 30-6-6 = 18

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
