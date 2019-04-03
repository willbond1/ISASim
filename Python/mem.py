import math
from cpu import CPU

class Line:
    dirty = False
    age = 0
    tag = 0

    # words is number of words in line
    def __init__(self, words, is_ram):
        mem_array = bytearray(words * (CPU.word_size // 8))
        empty = (not is_ram)

class Set:
    def __init__(ways, words, is_ram):
        lines = [Line(words, is_ram)] * ways
        if is_ram:
            for i in range(len(lines)):
                lines[i].tag = i

class Memory: # generic memory unit (cache or RAM)
    def __init__(latency, ways, size, line_length, is_ram):
        self.latency = latency # cycles to access memory
        self.ways = ways # lines per set
        self.size = size # size in bytes
        self.line_length = line_length # length of line in bytes

        self.line_n = size // line_length # number of lines
        self.set_n = math.ceil(line_n / ways) # number of sets
        self.index_n = math.ceil(math.log(self.set_n, 2)) # number of bits required to index sets
        self.word_n = (line_length * 8) // CPU.word_size # words per line