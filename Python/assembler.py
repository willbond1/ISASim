from mem import Cache, RAM
from cpu import CPU


# noinspection SpellCheckingInspection
class Assembler:
    condition_code = {  "EQ": 0b0000,
                        "NE": 0b0001,
                        "CS": 0b0010,
                        "CC": 0b0011,
                        "MI": 0b0100,
                        "PL": 0b0101,
                        "VS": 0b0110,
                        "VC": 0b0111,
                        "HI": 0b1000,
                        "LS": 0b1001,
                        "GE": 0b1010,
                        "LT": 0b1011,
                        "GT": 0b1100,
                        "LE": 0b1101,
                        "AL": 0b1110}

    instruction_map = {"MOV": {"instruction_code": 0b00, "opcode": 0b1101},
                       "MVN": {"instruction_code": 0b00, "opcode": 0b1111},
                       "CMP": {"instruction_code": 0b00, "opcode": 0b1010},
                       "CMN": {"instruction_code": 0b00, "opcode": 0b1011},
                       "TEQ": {"instruction_code": 0b00, "opcode": 0b1001},
                       "TST": {"instruction_code": 0b00, "opcode": 0b1000},
                       "AND": {"instruction_code": 0b00, "opcode": 0b0000},
                       "EOR": {"instruction_code": 0b00, "opcode": 0b0001},
                       "SUB": {"instruction_code": 0b00, "opcode": 0b0010},
                       "RSB": {"instruction_code": 0b00, "opcode": 0b0011},
                       "ADD": {"instruction_code": 0b00, "opcode": 0b0100},
                       "ADC": {"instruction_code": 0b00, "opcode": 0b0101},
                       "SBC": {"instruction_code": 0b00, "opcode": 0b0110},
                       "RSC": {"instruction_code": 0b00, "opcode": 0b0111},
                       "ORR": {"instruction_code": 0b00, "opcode": 0b1100},
                       "LDR": {"instruction_code": 0b01, "opcode": 0b0000},
                       "STR": {"instruction_code": 0b01, "opcode": 0b0000},
                       "BRX": {"instruction_code": 0b10, "opcode": 0b10},
                       "BRC": {"instruction_code": 0b10, "opcode": 0b01}}

    register_map = {"R1": 0b0001,
                    "R2": 0b0001,
                    "R3": 0b0001,
                    "R4": 0b0001,
                    "R5": 0b0001,
                    "R6": 0b0001,
                    "R7": 0b0001,
                    "R8": 0b0001,
                    "R9": 0b0001,
                    "R10": 0b0001,
                    "R11": 0b0001,
                    "R12": 0b0001,
                    "R13": 0b0001}


    def __init__(self, file):
        self.symbol_table = {}
        self.file = file
        self.outfile = open("program.txt", "w")
        self.machine_lines = []
        #for line in open(file, "r"):
            #self.machine_lines += [self.assemble(line)]

            # decode instruction in fetch_stage and return instruction fields as list


    def assemble_control(self, command, instruction):
        condition = 0b1110
        link = 0b0
        offset = 0b0
        if command.split()[0][3:5] in self.condition_code:
            condition = self.condition_code[command.split()[0][3:5]]
            if command.split()[0][5:6] == "L":
                link = 0b1
        elif command.split()[0][3:4] == "L":
            link = 0b1
        arg = command.split()[1]
        machine_code = condition << 28
        machine_code |= instruction["instruction_code"] << 26
        machine_code |= instruction["opcode"] << 24
        if instruction["opcode"] == 0b10:
            machine_code |= link << 23
            machine_code |= arg
        elif instruction["opcode"] == 0b01:
            machine_code |= link << 22
        if arg[0] == "R":
            machine_code |= self.register_map[arg]

    def extend(self, val, sign, word_length=32):
        bits = bin(val)[2:]
        bit_len = val.bit_len()
        if sign:
            prefix = bits[0] * (word_length - bit_len)
        else:
            prefix = '0' * (word_length - bit_len)

        return int(prefix + bits) & word_mask


    def assemble(self, command):
        instruction = self.instruction_map[command.split()[0][:3]]
        if instruction["instruction_code"] == 0b10:
            return self.assemble_control(command, instruction)
        if command.split()[0][3:5] in self.condition_code:
            condition = self.condition_code[command.split()[0][3:5]]

        instruction = self.instruction_map[command[:3]]

        print( condition )

        # def decode_ALU():
        #     I = (self.fetch_stage & 0x2000000) >> 25
        #     opcode = (self.fetch_stage & 0x1E00000) >> 21
        #     S = (self.fetch_stage & 0x100000) >> 20
        #     rd = (self.fetch_stage & 0xF0000) >> 16
        #     ro = (self.fetch_stage & 0xF000) >> 12
        #
        #     if I == 0:
        #         shift_type = (self.fetch_stage & 0x60) >> 5
        #         T = (self.fetch_stage & 0x10) >> 4
        #         ro2 = (self.fetch_stage & 0xF)
        #
        #         if T == 0:
        #             shift_immediate = (self.fetch_stage & 0xF80) >> 7
        #             return (cond_code, inst_code, I, opcode, S, rd, ro, shift_immediate, shift_type, T, ro2)
        #         else:
        #             rs = (self.fetch_stage & 0xF00) >> 8
        #             return (cond_code, inst_code, I, opcode, S, rd, ro, rs, shift_type, T, ro2)
        #     else:
        #         rotate = (self.fetch_stage & 0xF00) >> 8
        #         operand_immediate = (self.fetch_stage & 0xFF)
        #
        #         return (cond_code, inst_code, I, opcode, S, rd, ro, rotate, operand_immediate)
        #
        # def decode_memory():
        #     I = (self.fetch_stage & 0x2000000) >> 25
        #     P = (self.fetch_stage & 0x1000000) >> 24
        #     U = (self.fetch_stage & 0x800000) >> 23
        #     W = (self.fetch_stage & 0x400000) >> 22
        #     L = (self.fetch_stage & 0x200000) >> 21
        #     rn = (self.fetch_stage & 0x1E0000) >> 17
        #     rd = (self.fetch_stage & 0x1E000) >> 13
        #
        #     if I == 0:
        #         immediate_offset = (self.fetch_stage & 0x1FFF)
        #         return (cond_code, inst_code, I, P, U, W, L, rn, rd, immediate_offset)
        #     else:
        #         offset_shift = (self.fetch_stage & 0x1F00) >> 8
        #         shift_type = (self.fetch_stage & 0xC0) >> 6
        #         ro = (self.fetch_stage & 0xF)
        #
        #         return (cond_code, inst_code, I, P, U, W, L, rn, rd, offset_shift, shift_type, ro)
        #
        # def decode_control():
        #     I = (self.fetch_stage & 0x2000000) >> 25
        #     if I == 0:
        #         L = (self.fetch_stage & 0x800000) >> 23
        #         ra = (self.fetch_stage & 0x1F)
        #
        #         return (cond_code, inst_code, I, L, ra)
        #     else:
        #         L = (self.fetch_stage & 0x1000000) >> 24
        #         offset = (self.fetch_stage & 0xFFFFFF)
        #
        #         return (cond_code, inst_code, I, L, offset)
        #
        # if inst_code == 0:  # ALU
        #     return decode_ALU()
        # elif inst_code == 1:  # Memory operation
        #     return decode_memory()
        # elif inst_code == 2:  # Control operation
        #     return decode_control()
        # elif inst_code == 3:  # No op
        #     return (0, 3)
        # else:
        #     print('Error decoding instruction')
        #     return (empty_stage,)



        

print(Assembler("assembly_program.txt").assemble("BRXL R2"))
print(Assembler("assembly_program.txt").assemble("BRC R2"))
print(Assembler("assembly_program.txt").assemble("BRCL R2"))
print(Assembler("assembly_program.txt").assemble("BRCEQL 1000"))
