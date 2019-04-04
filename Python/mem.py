from math import ceil, log
from cpu import CPU

# represents single line/block in cache
class Line:
    dirty = False
    age = 0
    tag = 0

    # words is number of words in line
    def __init__(self, words, is_ram):
        self.mem_array = bytearray(words * CPU.word_size)
        self.empty = (not is_ram)

    def write(self, word, offset):
        self.mem_array[offset : (offset + CPU.word_size)] = word
        self.empty = False
    
    def write(self, words):
        self.mem_array = words
        self.empty = False
    
    def read(self, offset):
        return self.mem_array[offset : (offset + CPU.word_size)]
    
    def read(self):
        return self.mem_array
    
    def is_hit(self, tag):
        return (self.tag == tag and not empty)

    def evict(self):
        tmp = self.mem_array
        self.mem_array = bytearray(len(self.mem_array))
        self.empty = True
        return tmp

# represents single set which contains multiple lines
class Set:
    def __init__(self, ways, words, is_ram):
        self.lines = [Line(words, is_ram)] * ways
        if is_ram:
            for i in range(len(lines)):
                self.lines[i].tag = i

    def find_LRU(self):
        oldest = self.lines[0]
        for line in self.lines:
            if line.empty:
                return line
            if line.age > oldest.age:
                oldest = line
        
        return line

    def read(self, tag):
        for line in self.lines:
            if line.is_hit(tag):
                line.age = 0
                return line.read()

    def read(self, tag, offset):
        for line in self.lines:
            if line.is_hit(tag):
                line.age = 0
                return line.read(offset)
    
    def write(self, block, tag):
        for line in self.lines:
            if line.is_hit(tag):
                line.age = 0
                line.write(block)
                return
    
    def write(self, word, tag, offset):
        for line in self.lines:
            if line.is_hit(tag):
                line.age = 0
                line.write(word, offset)
                return

    # increment age of all blocks in set except given block
    def LRU_incr(self, recent_line):
        for line in self.lines:
            if line is not recent_line:
                line.age += 1
    
    def is_hit(self, tag):
        for line in self.lines:
            if line.is_hit(tag):
                return True
        return False

# generic memory class
class Memory:
    timers = {}

    def __init__(self, latency, ways, size, line_length, is_ram):
        self.latency = latency # cycles to access memory
        self.ways = ways # lines per set
        self.size = size # total size of memory in bytes
        self.line_length = line_length # size of line in bytes

        self.line_n = size // line_length # number of lines
        self.set_n = ceil(line_n / ways) # number of sets
        self.index_n = ceil(log(self.set_n, 2)) # number of bits required to index sets
        self.word_n = self.line_length // CPU.word_size # words per line
        self.offset_n = ceil(log(self.word_n, 2)) # number of bits in offset field
        self.tag_n = (CPU.word_size * 8) - (self.offset_n + self.index_n) # number of bits in tag field
        self.sets = [Set(self.ways, self.word_n, is_ram)] * self.set_n # create list of sets

    def index(addr):
        mask = (self.set_n - 1) << self.offset_n
        return ((addr & mask) >> self.offset_n)
    
    def tag(addr):
        mask = ((2 ** self.tag_n) - 1) << (self.offset_n + self.index_n)
        return ((addr & mask) >> (self.offset_n + self.index_n))
    
    def offset(addr):
        return (addr & (self.word_n - 1))

    # print memory stats for debug
    def print_stats():
        pass
    
    # display [size] bytes of memory starting at [addr]
    def display(addr, size):
        pass
    
    # read the word starting at [addr]
    def read(addr):
        self.f_cpu.clock += 1
    
    # write [word] to [addr]
    # word is a bytearray
    def write(addr, word):
        pass

    # read until it goes through
    def read_complete(addr):
        pass
    
    # attempt write until it goes through
    def write_complete(addr, word):
        pass

# cache class
class Cache(Memory):
    def __init__(self, latency, ways, size, line_length):
        super().__init__(latency, ways, size, line_length, False)

# memory class
class RAM(Memory):
    def __init__(self, latency, size, line_length):
        super().__init__(latency, 1, size, line_length, True)
    
    def display(addr, size):
        pass