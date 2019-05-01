# fetch stage: store next fetched instruction
# decode stage: store decoded instruction fields
# execute stage: store result of instruction execution
# memory stage: store result to be written back after memory operation
# writeback stage: store result of writeback
# forwarding register: dict that stores writeback values before writeback (register number -> value)
# None indicates empty

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

        self.registers = [0] * 16
        self.registers[PC] = 0
        self.registers[SP] = 0
        self.forward_register = {}

        self.memory_fetching = False
        self.memory_accessing = False # to ensure only one step can access memory at a time

        self.clock = 0
        self.memory = None

        # pipeline
        self.fetch_stage = None
        self.decode_stage = None # format: tuple that looks like (cond_code, inst_code, the rest)
        self.execute_stage = None
        self.memory_stage = None
        self.writeback_stage = None

        # control registers
        self.execute_store = None # not used for control. simply stores instruction tuple associated with result being held in execution stage
        self.memory_control = None
        self.writeback_control = None

    def display_cpu(self):
        for i in range(13):
            print("Contents of register", str(i) + ":" , hex(self.registers[i]) if self.registers[i] else hex(empty_reg))

        print("Contents of PC:", hex(self.registers[PC]) if self.registers[PC] else hex(empty_reg))
        print("Contents of LR:", hex(self.registers[LR]) if self.registers[LR] else hex(empty_reg))
        print("Contents of SP:", hex(self.registers[SP]) if self.registers[SP] else hex(empty_reg), "\n")

        print("N flag:", self.N)
        print("Z flag:", self.Z)
        print("C flag:", self.C)
        print("V flag:", self.V, "\n")

        print("Pipeline contents:")
        print("Fetch step:", hex(self.fetch_stage) if self.fetch_stage else hex(empty_stage))
        print("Decode step:", [hex(d) for d in self.decode_stage] if self.decode_stage else hex(empty_stage))

        if isinstance(self.execute_stage, tuple):
            print('Execute step:', [hex(i) if i else hex(empty_stage) for i in self.execute_stage])
        else:
            print("Execute step:", hex(self.execute_stage) if self.execute_stage else hex(empty_stage))

        if isinstance(self.memory_stage, tuple):
            print('Memory step:', [hex(i) if i else hex(empty_stage) for i in self.memory_stage])
        else:
            print("Memory step:", hex(self.memory_stage) if self.memory_stage else hex(empty_stage))

        if isinstance(self.writeback_stage, tuple):
            print('Writeback step:', [hex(i) if i else hex(empty_stage) for i in self.writeback_stage])
        else:
            print("Writeback step:", hex(self.writeback_stage) if self.writeback_stage else hex(empty_stage), '\n')
        
        print("Execute store:", [hex(d) for d in self.execute_store] if self.execute_store else hex(empty_stage))
        print("Memory control:", [hex(d) for d in self.memory_control] if self.memory_control else hex(empty_stage))
        print("Writeback control:", [hex(d) for d in self.writeback_control] if self.writeback_control else hex(empty_stage))

        print('CPU clock:', int(self.clock))

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
    # returns tuple consisting of shift result and last bit shifted out (for setting C flag)
    def shifter(self, value, amount, shift_type, word_length=32):
        if amount == 0:
            return (value, 0)

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

    # interpret binary as signed integer
    def as_signed(self, val, bit_len=26):
        max_val = (2 ** bit_len) # max unsigned value
        signed_max = (2 ** (bit_len - 1)) - 1 # max signed value
        return (val - max_val) if (val > signed_max) else val

    # grab next instruction from memory
    def fetch(self, active_memory):
        if not self.memory_accessing: # if there is not a memory operation accessing memory
            next_inst = active_memory.read(self.registers[PC])
            if next_inst: # memory read completed
                self.memory_fetching = False
                self.registers[PC] += 4
                next_inst = int.from_bytes(next_inst, byteorder='big', signed=False) # convert from bytearray to int
                return next_inst & word_mask
            else:
                self.memory_fetching = True
                return None
        else:
            return None
    
    # decode instruction in fetch_stage and return instruction fields as list
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
            return (0, 3)
        else:
            print('Error decoding instruction')
            return (None,)

    # execute instruction in decode_stage
    def execute(self):
        cond_code = self.decode_stage[0]
        inst_code = self.decode_stage[1]

        def execute_ALU():
            I, opcode, S, rd, ro = self.decode_stage[2:7]
            if ro in self.forward_register: # there is a forwarding value
                ro = self.forward_register[ro]
            else: # register not in dict, so current written back value is safe to use
                ro = self.registers[ro] & word_mask

            if I == 0:
                shift_type, T, ro2 = self.decode_stage[8:]
                if ro2 in self.forward_register:
                    ro2 = self.forward_register[ro2]
                else:
                    ro2 = self.registers[ro2] & word_mask
                
                if T == 0:
                    shift_immediate = self.decode_stage[7]
                    ro2, last_bit = self.shifter(ro2, shift_immediate, shift_type)
                else:
                    rs = self.decode_stage[7]
                    if rs in self.forward_register:
                        rs = self.forward_register[rs]
                    else:
                        rs = self.registers[rs] & word_mask
                    
                    ro2, last_bit = self.shifter(ro2, rs, shift_type)
            else:
                rotation, operand_immediate = self.decode_stage[7:]
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
                self.forward_register[rd] = result # record result in forwarding register
                return result
            else:
                return 0

        # returns a tuple consisting of the address to use for the memory access and the address to store in register
        def execute_memory():
            I, P, U, W, L, rn, rd = self.decode_stage[2:9]
            if I == 0:
                offset = self.decode_stage[9]
            else:
                offset_shift, shift_type, ro = self.decode_stage[9:]
                if ro in self.forward_register:
                    ro = self.forward_register[ro]
                else:
                    ro = self.registers[ro] & word_mask

                offset = self.shifter(ro, offset_shift, shift_type)
            
            if rn in self.forward_register:
                rn_val = self.forward_register[rn]
            else:
                rn_val = self.registers[rn] & word_mask

            before_addr = rn_val
            if U == 0: # how is offset applied?
                after_addr = before_addr - offset
            else:
                after_addr = before_addr + offset
            
            if P == 0: # pre or post indexing?
                use_addr = before_addr
            else:
                use_addr = after_addr
            
            if W == 0: # writeback
                reg_addr = 0
            else:
                reg_addr = after_addr
                self.forward_register[rn] = reg_addr
        
            return (use_addr, reg_addr)

        def execute_control_method():
            I, L = self.decode_stage[2:4]
            if I:
                offset = self.decode_stage[4]
                offset = self.as_signed(offset << 2)
                target = self.registers[PC] + offset
            else:
                ra = self.decode_stage[4]
                if ra in self.forward_register:
                    ra = self.forward_register[ra]
                else:
                    ra = self.registers[ra] & word_mask
                target = ra
            
            if L:
                self.registers[LR] = self.registers[PC] - 4
            
            # flush first two stages since branch is taken
            self.fetch_stage = None
            self.decode_stage = None
            self.registers[PC] = target
            return target

        if self.cond(cond_code):
            if inst_code == 0:
                return execute_ALU()
            elif inst_code == 1:
                return execute_memory()
            elif inst_code == 2:
                return execute_control_method()
            elif inst_code == 3:
                return word_mask
            else:
                print('Error executing instruction')
                return None
        else:
            return word_mask

    # if instruction is a memory instruction, perform operation using use address
    # returns (read writeback, address writeback)
    def memory_inst(self, active_memory):
        inst_code = self.memory_control[1]
        if inst_code == 1 and self.execute_stage != word_mask:
            use_addr, write_addr = self.execute_stage
            use_addr += self.registers[SP] # read/write relative to stack pointer
            L = self.memory_control[6]
            W = self.memory_control[5]
            rd = self.memory_control[8]

            if L == 0: # L is 0 for write
                if not self.memory_fetching: # check if memory is busy with fetching instruction
                    self.memory_accessing = True
                    if rd in self.forward_register:
                        rd = self.forward_register[rd]
                    else:
                        rd = self.registers[rd] & word_mask

                    if active_memory.write(use_addr, rd.to_bytes(4, byteorder='big')): # write went through, so memory is freed up
                        self.memory_accessing = False
                        if W: # W == 1 for writeback
                            return (None, write_addr)
                        else:
                            return (None, None)
                    else: # still writing
                        return None
                else: # can't write
                    self.memory_accessing = False
                    return None

            else: # handle read
                if not self.memory_fetching:
                    self.memory_accessing = True
                    read_word = active_memory.read(use_addr)

                    if read_word: # read went through
                        self.memory_accessing = False
                        self.forward_register[rd] = int.from_bytes(read_word, byteorder='big')
                        if W:
                            return (int.from_bytes(read_word, byteorder='big'), write_addr)
                        else:
                            return (int.from_bytes(read_word, byteorder='big'), None)
                    else:
                        return None
                else:
                    self.memory_accessing = False
                    return None

        else: # instruction is not memory access, so execute stage simply contains operation result
            return self.execute_stage

    # write instruction result to destination register (if there is one) handling different instruction types
    def writeback(self):
        if self.memory_stage: # indicates writeback
            inst_code = self.writeback_control[1]
            if inst_code == 0: # ALU
                rd = self.writeback_control[5]
                self.registers[rd] = self.memory_stage
                self.forward_register.pop(rd, None) # remove register from forwarding register since writeback has completed

            elif inst_code == 1: # memory
                rn, rd = self.writeback_control[7:9]
                read_writeback, addr_writeback = self.memory_stage

                if read_writeback:
                    self.registers[rd] = read_writeback
                    self.forward_register.pop(rd, None)

                if addr_writeback:
                    self.registers[rn] = addr_writeback
                    self.forward_register.pop(rn, None)

            elif (inst_code == 2) or (inst_code == 3): # control and no-op instructions don't write back
                return word_mask
            else:
                print('Error finding destination register in writeback stage')
                return None

            return self.memory_stage

        else: # value to write back is None, indicating no writeback
            return 0

    # step pipeline
    def step(self, with_cache, with_pipe):
        active_memory = self.memory
        if not with_cache: # get reference to RAM (no next level)
            while active_memory.next_level:
                active_memory = active_memory.next_level

        self.writeback_stage = None
        self.writeback_control = None

        # move from memory to writeback stage, checking to see if pipeline is blocked
        wrote_back = False
        if (not self.writeback_stage and not self.writeback_control and self.memory_stage and self.memory_control):
            self.writeback_control = self.memory_control
            self.writeback_stage = self.writeback()
            wrote_back = True
            self.memory_control = None
            self.memory_stage = None

        # move from execute to memory stage
        if (not self.memory_stage and self.execute_stage):
            if not self.memory_control: # since memory operation can stall, if control register is empty, move it up
                self.memory_control = self.execute_store
                self.execute_store = None

            mem_result = self.memory_inst(active_memory) # attempt memory instruction
            if mem_result:
                self.memory_stage = mem_result
                self.execute_stage = None

        # move from decode to control registers/execute stage
        if (not self.execute_store and not self.execute_stage and self.decode_stage):
            self.execute_stage = self.execute()
            self.execute_store = self.decode_stage
            self.decode_stage = None

        # move from fetch to decode stage
        if (not self.decode_stage and self.fetch_stage):
            self.decode_stage = self.decode()
            self.fetch_stage = None
        
        # move instruction from memory to fetch stage
        if (with_pipe or wrote_back):
            next_inst = self.fetch(active_memory)
            if next_inst:
                self.fetch_stage = next_inst
        
        self.clock += 1
        return True
    
    def read(self, addr):
        return int.from_bytes(self.memory.read_complete(addr), byteorder='big')
    
    def write(self, addr, word):
        self.memory.write_complete(addr, word.to_bytes(4, byteorder='big'))

empty_stage = word_mask = int(('FF' * CPU.word_size), 16)
empty_reg = int(('00' * CPU.word_size), 16)

from mem import Cache, RAM