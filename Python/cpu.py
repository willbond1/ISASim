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
        self.memory = None

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

    def set_memory(self, memory):
        self.memory = memory

    # determine truthiness of condition code
    def cond(self, cond_code):
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
    def shifter(self, value, amount, shift_type, word_length=32):
        if shift_type == 0:
            result = (value << amount) & word_mask
            last_bit = (value >> (word_length - amount)) & 0x1
        elif shift_type == 1:
            result = ((value >> amount) if value >= 0 else ((value + (1 << word_length)) >> amount)) & word_mask # 0x100000000 = 1 << 32
            last_bit = (value >> (amount - 1)) & 0x1
        elif shift_type == 2:
            result = (value >> amount) & word_mask
            last_bit = (value >> (amount - 1)) & 0x1
        elif shift_type == 3:
            result = (((value >> amount) | (value << (word_length - amount)))) & word_mask
            last_bit = (value >> (amount - 1)) & 0x1
        else:
            result = value
            last_bit = 0
        
        return (result, last_bit)

    # extend val with sign if sign is true, with 0 if false
    def extend(self, val, sign, word_length=32):
        bits = bin(val)[2:]
        bit_len = val.bit_len()
        if sign:
            prefix = bits[0] * (word_length - bit_len)
        else:
            prefix = '0' * (word_length - bit_len)
        
        return int(prefix + bits) & word_mask

    # grab next instruction from memory
    def fetch(self, active_memory):
        next_inst = active_memory.read_complete(self.registers[PC])
        self.registers[PC] += 4
        return next_inst & word_mask
    
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
                shift_type = (self.fetch_stage & 0x60) >> 5
                T = (self.fetch_stage & 0x10) >> 4
                ro2 = (self.fetch_stage & 0xF)

                if T == 0:
                    shift_immediate = (self.fetch_stage & 0xF80) >> 7
                    return (cond_code, inst_code, I, opcode, S, rd, ro, shift_immediate, shift_type, T, ro2)
                else:
                    rs = (self.fetch_stage & 0xF00) >> 8
                    return (cond_code, inst_code, I, opcode, S, rd, ro, rs, shift_type, T, ro2)
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
            I, opcode, S, rd, ro = self.execute_control[2:7]
            ro = self.registers[ro] & word_mask
            
            if I == 0:
                shift_type, T, ro2 = self.execute_control[8:]
                if T == 0:
                    shift_immediate = self.execute_control[7]
                    ro2, last_bit = self.shifter(ro2, shift_immediate, shift_type)
                else:
                    rs = self.execute_control[7]
                    ro2, last_bit = self.shifter(ro2, self.registers[rs], shift_type)
            else:
                rotation, operand_immediate = self.execute_control[7:]
                operand_immediate = self.extend(operand_immediate, False)
                ro2, last_bit = self.shifter(operand_immediate, (rotation * 2), 3)
            
            if opcode == 0: # AND
                result = ro & ro2
                written = True
            elif opcode == 1: # EOR
                result = ro ^ ro2
                written = True
            elif opcode == 2: # SUB
                result = ro - ro2
                written = True
            elif opcode == 3: # RSB
                result = ro2 - ro
                written = True
            elif opcode == 4: # ADD
                result = ro + ro2
                written = True
            elif opcode == 5: # ADC
                result = ro + ro2 + int(self.C)
                written = True
            elif opcode == 6: # SBC
                result = ro - ro2 + int(self.C) - 1
                written = True
            elif opcode == 7: # RSC
                result = ro2 - ro + int(self.C) - 1
                written = True
            elif opcode == 8: # TST
                result = ro & ro2
                written = False
            elif opcode == 9: # TEQ
                result = ro ^ ro2
                written = False
            elif opcode == 10: # CMP
                result = ro - ro2
                written = False
            elif opcode == 11: # CMN
                result = ro + ro2
                written = False
            elif opcode == 12: # ORR
                result = ro | ro2
                written = True
            elif opcode == 13: # MOV
                result = ro2
                written = True
            elif opcode == 14: # BIC
                result = ro & (~ro2)
                written = True
            elif opcode == 15: # MVN
                result = ~ro2
                written = True
            else:
                print('Error finding ALU opcode')
                return None
            
            # handle setting condition registers
            if S:
                trunc_result = result & word_mask
                self.N = (result < 0)
                self.Z = (result == 0)
                self.V = ((result > 0 and trunc_result < 0) or (result < 0 and trunc_result > 0))
            
                if opcode in (2, 3, 4, 5, 6, 7, 10, 11):
                    self.C = (abs(result) > abs(trunc_result))
                else:
                    self.C = bool(last_bit)
            
            if written:
                return result & word_mask
            else:
                return None
        
        # returns a tuple consisting of the address to use for the memory access and the address to store in register
        def execute_memory():
            I, P, U, W, L, rn, rd = self.execute_control[2:9]
            if I == 0:
                offset = self.execute_control[9]
            else:
                offset_shift, shift_type, ro = self.execute_control[9:]
                offset = self.shifter(self.registers[ro], offset_shift, shift_type)
            
            before_addr = self.registers[rn]
            if U == 0: # how is offset applied?
                after_addr = before_addr - offset
            else:
                after_addr = before_addr + offset
            
            if P == 0: # pre or post indexing?
                use_addr = before_addr
            else:
                use_addr = after_addr
            
            if W == 0: # writeback?
                reg_addr = None
            else:
                reg_addr = after_addr
        
            return (use_addr, reg_addr)

        def execute_control():
            I, L = self.execute_control[2:4]
            if I:
                offset = self.execute_control[4]
                offset = self.extend((offset << 2), True)
                target = self.registers[PC] + offset
            else:
                ra = self.execute_control[4]
                target = self.registers[ra]
            
            if L:
                self.registers[LR] = self.registers[PC] - 4
            
            self.fetch_stage = empty_stage
            self.decode_stage = empty_stage
            self.registers[PC] = target
            return target
        
        if cond(cond_code):
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
        else:
            # flush pipeline
            self.fetch_stage = empty_stage
            self.decode_stage = empty_stage
            self.registers[PC] -= 4 # rewind PC to instruction after this one
            return None

    # if instruction is a memory instruction, perform operation using use address
    # returns write address
    def memory_inst(self, active_memory):
        inst_code = self.memory_control[1]
        if inst_code == 1:
            use_addr, write_addr = self.execute_stage
            L = self.memory_control[6]
            rd = self.memory_control[8]

            if L == 0:
                active_memory.write_complete(use_addr, self.registers[rd])
            else:
                self.registers[rd] = active_memory.read_complete(use_addr)
            
            return write_addr
        else: # instruction is not memory access, so execute stage simply contains operation result
            return self.execute_stage

    # write instruction result to destination register (if there is one) handling different instruction types
    def writeback(self):
        if self.memory_stage:
            inst_code = self.writeback_control[1]
            if inst_code == 0:
                rd = self.writeback_control[5]
            elif inst_code == 1:
                rd = self.writeback_control[8]
            elif (inst_code == 2) or (inst_code == 3): # control and no-op instructions don't write back
                return None
            else:
                print('Error finding destination register in writeback stage')
                return  

            self.registers[rd] = self.memory_stage
            return self.memory_stage
        else: # value to write back is None, indicating no writeback
            return None

    # step pipeline, returns true if program is continuing, false if ended
    def step(self, with_cache, with_pipe):
        self.clock += 1
        wrote_back = False
        active_memory = self.memory
        if not with_cache: # get reference to RAM (no next level)
            while active_memory.next_level:
                active_memory = active_memory.next_level

        if self.writeback_stage == 0: # 0 instruction signals end of program
            return False
        self.writeback_stage = empty_stage
        self.writeback_control = empty_reg

        writeback_block = (self.memory_stage == empty_stage or self.writeback_stage != empty_stage or self.memory_control == empty_reg or self.writeback_control != empty_reg)
        if not writeback_block: # move from memory to writeback stage
            self.writeback_control = self.memory_control
            self.writeback_stage = self.writeback()
            wrote_back = True
            self.memory_control = empty_reg
            self.memory_stage = empty_stage
        
        fetch_block = (not with_pipe and not wrote_back)

        memory_block = (self.execute_stage == empty_stage or self.memory_stage != empty_stage or self.execute_control == empty_reg or self.memory_control != empty_reg)
        if not memory_block: # move from execute to memory stage
            self.memory_control = self.execute_control
            self.memory_stage = self.memory_inst(active_memory)
            self.execute_control = empty_reg
            self.execute_stage = empty_stage
        
        execute_block = (self.decode_stage == empty_stage or self.execute_stage != empty_stage or self.execute_control != empty_reg)
        if not execute_block: # move from decode to control registers/execute stage
            self.execute_control = self.decode_stage
            self.execute_stage = self.execute()
            self.decode_stage = empty_stage
        
        decode_block = (self.fetch_stage == empty_stage or self.decode_stage != empty_stage)
        if not decode_block: # move from fetch to decode stage
            self.decode_stage = self.decode()
            self.fetch_stage = empty_stage
        
        if not fetch_block: # move instruction from memory to fetch stage
            self.fetch_stage = self.fetch(active_memory)
        
        return True
    
    def read(self, addr):
        return self.memory.read_complete(addr)
    
    def write(self, addr, word):
        self.memory.write_complete(addr, word)

empty_stage = word_mask = int(('FF' * CPU.word_size), 16)
empty_reg = int(('00' * CPU.word_size), 16)

from mem import Cache, RAM