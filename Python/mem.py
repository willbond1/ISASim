# represents single line/block in cache
class Line:
    dirty = False
    age = 0
    tag = 0x00

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
        return (self.tag, self.mem_array)
    
    def is_hit(self, tag):
        return (self.tag == tag and not empty)
    
    def display(self):
        print('Tag: ', hex(self.tag), ' Age: ', self.age, ' Line: ', end='')
        print(' '.join('{:02x}'.format(x) for x in self.mem_array))

    def RAM_display(self, addr):
        print(hex(addr), '   ', end='')
        print(' '.join('{:02x}'.format(x) for x in self.mem_array))

# represents single set which contains multiple lines
class Set:
    def __init__(self, ways, words, is_ram):
        self.lines = [Line(words, is_ram)] * ways
        if is_ram:
            for i in range(len(self.lines)):
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
                return line.read()

    def read(self, tag, offset):
        for line in self.lines:
            if line.is_hit(tag):
                return line.read(offset)
    
    def write(self, block, tag):
        for line in self.lines:
            if line.is_hit(tag):
                line.write(block)
                return
    
    def write(self, word, tag, offset):
        for line in self.lines:
            if line.is_hit(tag):
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
                return line
        return None

    def display(self):
        for line in self.lines:
            line.display()

# generic memory class
class Memory:
    timers = {}

    def __init__(self, latency, ways, size, line_length, is_ram):
        self.latency = latency # cycles to access memory
        self.ways = ways # lines per set
        self.size = size # total size of memory in bytes
        self.line_length = line_length # size of line in bytes
        self.next_level = None
        self.f_cpu = None

        self.line_n = self.size // self.line_length # number of lines
        self.set_n = ceil(self.line_n / ways) # number of sets
        self.index_n = ceil(log(self.set_n, 2)) # number of bits required to index sets
        self.word_n = self.line_length // CPU.word_size # words per line
        self.offset_n = ceil(log(self.word_n, 2)) # number of bits in offset field
        self.tag_n = (CPU.word_size * 8) - (self.offset_n + self.index_n) # number of bits in tag field
        self.sets = [Set(self.ways, self.word_n, is_ram)] * self.set_n # create list of sets

    def set_CPU(self, processor):
        self.f_cpu = processor

    def set_next_level(self, next_level):
        self.next_level = next_level

    def index(self, addr):
        mask = (self.set_n - 1) << self.offset_n
        return ((addr & mask) >> self.offset_n)
    
    def tag(self, addr):
        mask = ((2 ** self.tag_n) - 1) << (self.offset_n + self.index_n)
        return ((addr & mask) >> (self.offset_n + self.index_n))
    
    def offset(self, addr):
        return (addr & (self.word_n - 1))

    def encode(self, tag, index, offset):
        addr = tag << self.index_n
        addr |= index
        addr <<= self.offset_n
        addr |= offset
        return addr

    # print memory stats for debug
    def print_stats(self):
        print('Latency: ', self.latency)
        print('Size in bytes: ', self.size)
        print('Line/block length in bytes: ', self.line_length)
        print('Associativity: ', self.ways)
        print('Number of lines: ', self.line_n)
        print('Number of sets: ', self.set_n)
        print('Words per line: ', self.word_n)
        print('Number of bits in index field: ', self.index_n)
        print('Number of bits in offset field: ', self.offset_n)
        print('Number of bits in tag field: ', self.tag_n)
    
    # display [size] sets of memory starting at [addr]
    def display(self, addr, size):
        index = index(addr)
        if (index + size) > self.set_n:
            print('Out of range')
            return
        for i in range(index, (index + size)):
            print('Set index: ', hex(i))
            self.sets[i].display()
    
    def write_block(self, addr, block):
        self.timers.setdefault(addr, default=0)
        while self.timers[addr] < self.latency:
            self.f_cpu.clock += 1
            self.timers[addr] += 1
        self.timers[addr] = 0

        tag = tag(addr)
        offset = offset(addr)
        index = index(addr)

        hit_block = self.sets[index].is_hit(tag)
        if not hit_block:
            hit_block = self.sets[index].find_LRU()
            if hit_block.dirty:
                old_tag, old_data = hit_block.read()
                old_addr = self.encode(old_tag, index, 0)
                self.next_level.write_block(old_addr, old_data)

        hit_block.write(block)
        hit_block.tag = tag
        self.sets[index].LRU_incr(hit_block)
        hit_block.age = 0
        hit_block.dirty = True
        self.f_cpu.clock += 1

    def read_block(self, addr):
        self.timers.setdefault(addr, default=0)
        while self.timers[addr] < self.latency:
            self.f_cpu.clock += 1
            self.timers[addr] += 1
        self.timers[addr] = 0

        tag = tag(addr)
        offset = offset(addr)
        index = index(addr)

        hit_block = self.sets[index].is_hit(tag)
        if not hit_block:
            hit_block = self.sets[index].find_LRU()
            if hit_block.dirty:
                old_tag, old_data = hit_block.read()
                old_addr = self.encode(old_tag, index, 0)
                self.next_level.write_block(old_addr, old_data)
            
            new_tag, new_data = self.next_level.read_block(addr)
            hit_block.write(new_data)
            hit_block.tag = new_tag
            hit_block.dirty = False
        
        hit_block.age = 0
        self.sets[index].LRU_incr(hit_block)
        self.f_cpu.clock += 1
        return hit_block.read()

    # read the word starting at [addr]
    def read(self, addr):
        t = self.timers.setdefault(addr, default=0)

        if t == self.latency:
            self.timers[addr] = 0
            tag = tag(addr)
            offset = offset(addr)
            index = index(addr)

            hit_block = self.sets[index].is_hit(tag)
            if not hit_block:
                hit_block = self.sets[index].find_LRU()
                if hit_block.dirty:
                    old_tag, old_data = hit_block.read()
                    old_addr = self.encode(old_tag, index, 0)
                    self.next_level.write_block(old_addr, old_data)
                
                new_tag, new_data = self.next_level.read_block(addr)
                hit_block.write(new_data)
                hit_block.tag = new_tag
                hit_block.dirty = False

            hit_block.age = 0
            self.sets[index].LRU_incr(hit_block)
            self.f_cpu.clock += 1
            return hit_block.read(offset)

        else:
            self.timers[addr] += 1
            self.f_cpu.clock += 1
            return None
    
    # write [word] to [addr]
    # word is a bytearray
    def write(self, addr, word):
        t = self.timers.setdefault(addr, default=0)

        if t == self.latency:
            self.timers[addr] = 0
            tag = tag(addr)
            offset = offset(addr)
            index = index(addr)

            hit_block = self.sets[index].is_hit(tag)
            if not hit_block:
                hit_block = self.sets[index].find_LRU()
                if hit_block.dirty:
                    old_tag, old_data = hit_block.read()
                    old_addr = self.encode(old_tag, index, 0)
                    self.next_level.write_block(old_addr, old_data)
                
                new_tag, new_data = self.next_level.read_block(addr)
                hit_block.write(new_data)
                hit_block.tag = new_tag
                hit_block.dirty = False
            
            hit_block.write(word, offset)
            hit_block.age = 0
            self.sets[index].LRU_incr(hit_block)
            hit_block.dirty = True
            self.f_cpu.clock += 1

        else:
            self.timers[addr] += 1
            self.f_cpu.clock += 1

    # read until it goes through
    def read_complete(self, addr):
        while self.timers[addr] < self.latency:
            data = self.read(addr)
        return self.read(addr)
    
    # attempt write until it goes through
    def write_complete(self, addr, word):
        while self.timers[addr] < self.latency:
            self.write(addr, word)
        self.write(addr, word)

# cache class
class Cache(Memory):
    def __init__(self, latency, ways, size, line_length):
        super().__init__(latency, ways, size, line_length, False)

# memory class
class RAM(Memory):
    def __init__(self, latency, size, line_length):
        super().__init__(latency, 1, size, line_length, True)
    
    # display [size] lines of RAM starting at [addr]
    def display(self, addr, size):
        index = index(addr)
        if (index + size) > self.set_n:
            print('Out of range')
            return
        for i in range(index, (index + size)):
            self.sets[i].RAM_display(i)

from math import ceil, log
from cpu import CPU