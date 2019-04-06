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
    
        # are stages blocked?
        self.fetch_block = False
        self.decode_block = False
        self.execute_block = False
        self.memory_block = False
        self.writeback_block = False

    def cond(cond_code):
        if cond_code == 0:
            return self.Z
        elif cond_code == 1:
            return (not self.Z)
        elif cond_code == 2:
            return self.C
        elif cond_code == 3:
            return (not self.C)
        elif cond_code == 4:
            return self.N
        elif cond_code == 5:
            return (not self.N)
        elif cond_code == 6:
            return self.V
        elif cond_code == 7:
            return (not self.V)
        elif cond_code == 8:
            return (self.C and (not self.Z))
        elif cond_code == 9:
            return ((not self.C) or self.Z)
        elif cond_code == 10:
            return (self.N == self.V)
        elif cond_code == 11:
            return (self.N != self.V)
        elif cond_code == 12:
            return ((not self.Z) and (self.N == self.V))
        elif cond_code == 13:
            return (self.Z or (self.N != self.V))
        else:
            return True

    # handle different types of shift
    def shifter(self, value, amount, shift_type):
        if shift_type == 0:
            return (value << amount)
        elif shift_type == 1:
            return (value >> amount) if value >= 0 else ((value + 0x100000000) >> amount) # 0x100000000 = 1 << 32
        elif shift_type == 2:
            return (value >> amount)
        elif shift_type == 3:
            return ((value >> amount) | (value << (32 - amount)))
        else:
            return value

    # grab next instruction from memory
    def fetch(self, active_memory):
        next_inst = active_memory.read(self.registers[PC])
        if next_inst:
            return next_inst
        else:
            return empty_stage
    
    # decode instruction in fetch_stage and return instruction fields as tuple
    def decode(self):
        cond_code = (self.fetch_stage & 0xF0000000) >> 28
        inst_code = (self.fetch_stage & 0xC000000) >> 26

        def decode_ALU():
            I = (self.fetch_stage & 0x2000000) >> 25
            opcode = (self.fetch_stage & 0x1E00000) >> 21
            S = (self.fetch_stage & 0x100000) >> 20
            rd = (self.fetch_stage & 0xF0000) >> 16
            ro = (self.fetch_stage & 0xF000) >> 12

            if I == 0:
                shift = (self.fetch_stage & 0xF80) >> 7
                shift_type = (self.fetch_stage & 0x60) >> 5
                T = (self.fetch_stage & 0x10) >> 4
                ro2 = (self.fetch_stage & 0xF)

                return (cond_code, inst_code, I, opcode, S, rd, ro, shift, shift_type, T, ro2)
            else:
                rotate = (self.fetch_stage & 0xF00) >> 8
                operand_immediate = (self.fetch_stage & 0xFF)

                return (cond_code, inst_code, I, opcode, S, rd, ro, rotate, operand_immediate)
        
        def decode_memory():
            I = (self.fetch_stage & 0x2000000) >> 25
            P = (self.fetch_stage & 0x1000000) >> 24
            U = (self.fetch_stage & 0x800000) >> 23
            W = (self.fetch_stage & 0x400000) >> 22
            L = (self.fetch_stage & 0x200000) >> 21
            rn = (self.fetch_stage & 0x1E0000) >> 17
            rd = (self.fetch_stage & 0x1E000) >> 13

            if I == 0:
                immediate_offset = (self.fetch_stage & 0x1FFF)
                return (cond_code, inst_code, I, P, U, W, L, rn, rd, immediate_offset)
            else:
                offset_shift = (self.fetch_stage & 0x1F00) >> 8
                shift_type = (self.fetch_stage & 0xC0) >> 6
                ro = (self.fetch_stage & 0xF)

                return (cond_code, inst_code, I, P, U, W, L, rn, rd, offset_shift, shift_type, ro)
        
        def decode_control():
            I = (self.fetch_stage & 0x2000000) >> 25
            if I == 0:
                L = (self.fetch_stage & 0x800000) >> 23
                ra = (self.fetch_stage & 0x1F)

                return (cond_code, inst_code, I, L, ra)
            else:
                L = (self.fetch_stage & 0x1000000) >> 24
                offset = (self.fetch_stage & 0xFFFFFF)

                return (cond_code, inst_code, I, L, offset)

        if inst_code == 0: # ALU
            return decode_ALU()
        elif inst_code == 1: # Memory operation
            return decode_memory()
        elif inst_code == 2: # Control operation
            return decode_control()
        elif inst_code == 3: # No op
            return (empty_stage,)
        else:
            print('Error decoding instruction')
            return (None,)

    # execute instruction in decode_stage
    def execute(self):
        cond_code = self.execute_control[0]
        inst_code = self.execute_control[1]

        def execute_ALU():
            I = self.execute_control[2]
            opcode = self.execute_control[3]
            S = self.execute_control[4]
            rd = self.execute_control[5]
            ro = self.execute_control[6]

            if I == 0:
                pass
            else:
                pass
        
        def execute_memory():
            pass
        
        def execute_control():
            pass
        
        if inst_code == 0:
            return execute_ALU()
        elif inst_code == 1:
            return execute_memory()
        elif inst_code == 2:
            return execute_control()
        elif inst_code == 3:
            return None
        else:
            print('Error executing instruction')
            return None

    # if instruction is a memory instruction, perform operation
    def memory_inst(self):
        pass

    # step pipeline, returns true if program is continuing, false if ended
    def step(with_cache, with_pipe):
        active_memory = self.memory
        if not with_cache: # get reference to RAM (no next level)
            while active_memory.next_level:
                active_memory = active_memory.next_level
        
        # move writeback stage out of pipeline

        self.writeback_block = (self.memory_stage == empty_stage or self.writeback_stage != empty_stage or self.memory_control == empty_reg or self.writeback_control != empty_reg)
        if not self.writeback_block: # move from memory to writeback stage
            pass
        
        self.memory_block = (self.execute_stage == empty_stage or self.memory_stage != empty_stage or self.execute_control == empty_reg or self.memory_control != empty_reg)
        if not self.memory_block: # move from execute to memory stage
            pass
        
        self.execute_block = (self.decode_stage == empty_stage or self.execute_stage != empty_stage or self.execute_control != empty_reg)
        if not self.execute_block: # move from decode to control registers/execute stage
            pass
        
        self.decode_block = (self.fetch_stage == empty_stage or self.decode_stage != empty_stage)
        if not self.decode_block: # move from fetch to decode stage
            pass
        
        if not self.fetch_block: # move instruction from memory to fetch stage
            self.fetch_stage = fetch(active_memory)
    
    def read(addr):
        self.clock += 1
        return self.memory.read_complete(addr)
    
    def write(addr, word):
        self.clock += 1
        self.memory.write_complete(addr, word)