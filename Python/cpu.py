from mem import Cache, RAM

empty_stage = int('0xFF' * CPU.word_size)
empty_reg = int('0x00' * CPU.word_size)

# register constants for convenience
SP = 13
LR = 14
PC = 15

class CPU:
    word_size = 4 # word size in bytes

    def __init__(self):
        self.N = False
        self.Z = False
        self.C = False
        self.V = False
        self.clock = 0
        self.registers = [empty_reg] * 16

        # pipeline
        self.fetch_stage = empty_stage
        self.decode_stage = empty_stage # format: tuple that looks like (cond_code, inst_code, the rest)
        self.execute_stage = empty_stage
        self.memory_stage = empty_stage
        self.writeback_stage = empty_stage

        # control registers
        self.execute_control = empty_reg
        self.memory_control = empty_reg
        self.writeback_control = empty_reg
    
    # grab next instruction from memory
    def fetch(self, active_memory):
        next_inst = active_memory.read(self.registers[PC])
        return next_inst
    
    # decode instruction in fetch_stage
    def decode(self):
        cond_code = (self.fetch_stage & 0xF0000000) >> 28
        inst_code = (self.fetch_stage & 0x0C000000) >> 26

        def decode_ALU():
            pass
        
        def decode_memory():
            pass
        
        def decode_control():
            pass

        if inst_code == 0: # ALU
            pass
        elif inst_code == 1: # Memory operation
            pass
        elif inst_code == 2: # Control operation
            pass
        elif inst_code == 3: # No op
            pass
        else:
            print('Error decoding instruction')
            return None

    # execute instruction in decode_stage
    def execute(self):
        pass

    # execute instruction in memory_stage
    def memory_inst(self):
        pass
    
    def writeback(self):
        pass
    
    # empty pipeline before execute stage and rewind PC
    def flush():
        pass

    # step pipeline, returns true if program is continuing, false if ended
    def step(with_cache, with_pipe):
        pass