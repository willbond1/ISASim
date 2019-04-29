from mem import Cache, RAM
from cpu import CPU
import re


# noinspection SpellCheckingInspection
class Assembler:
    shift_map = {       "LSL": 0b00,
                        "LSR": 0b01,
                        "ASR": 0b10,
                        "ROR": 0b11}

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
                        "AL": 0b1110,
                        "": 0b1110}

    instruction_map = {"MOV": {"instruction_code": 0b00, "opcode": 0b1101},
                       "BIC": {"instruction_code": 0b00, "opcode": 0b1110},
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
                       "LDR": {"instruction_code": 0b01, "opcode": 0b1},
                       "STR": {"instruction_code": 0b01, "opcode": 0b0},
                       "BRX": {"instruction_code": 0b10, "opcode": 0b01},
                       "BRC": {"instruction_code": 0b10, "opcode": 0b10}}

    register_map = {"R0": 0b0000,
                    "R1": 0b0001,
                    "R2": 0b0010,
                    "R3": 0b0011,
                    "R4": 0b0100,
                    "R5": 0b0101,
                    "R6": 0b0110,
                    "R7": 0b0111,
                    "R8": 0b1000,
                    "R9": 0b1001,
                    "R10": 0b1010,
                    "R11": 0b1011,
                    "R12": 0b1100,
                    "R13": 0b1101}

    offset_regex = re.compile("\[\s*(R\d\d?)\s*,\s*(\S.+)]\Z")
    pre_regex = re.compile("\[\s*(R\d\d?)\s*,\s*(\S.+)\s*]!\Z")
    post_regex = re.compile("\[\s*(R\d\d?)\s*\]\s*,\s*(\d+)\Z")

    immediate_regex = re.compile("#(\+|\-)?(\d+)\s*\Z")
    index_regex = re.compile("(\+|\-)?(R\d\d?)\s*\Z")
    shifted_index_regex = re.compile("(\+|\-)?(R\d\d?)\s*,\s*(\w{3})\s+#(\d+)\s*\Z")

    def __init__(self, file):
        self.symbol_table = {}
        self.file = file
        self.outfile = open("program.txt", "w")
        self.machine_lines = []
        for line in open(file, "r"):
            self.machine_lines += [str(bin(self.assemble(line))) + "\n"]
        self.outfile.writelines(self.machine_lines)
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
            machine_code |= link << 24
            machine_code |= (int(arg) & 0xffffff)
        elif instruction["opcode"] == 0b01:
            machine_code |= link << 23
            machine_code |= self.register_map[arg]
        return machine_code


    def assemble_memory(self, command, instruction):
        command = command.strip()
        condition = 0b1110
        offset = 0b0
        tokenized = re.findall("(LDR|STR)(\w{,2})\s*(R\d\d?)\s*,\s*(\S.+)", command)[0]
        condition = self.condition_code[tokenized[1]]
        rd = self.register_map[tokenized[2]]

        machine_code = condition << 28
        machine_code |= instruction["instruction_code"] << 26
        machine_code |= instruction["opcode"] << 21
        machine_code |= rd << 13

        op2 = tokenized[3]

        if re.match("R\d\d?\s*\Z", op2):
            rn = self.register_map[op2]
            machine_code |= rn << 17
            machine_code |= 0b1 << 24
            return machine_code

        offset = 0
        op2_tokenized = ""
        if self.offset_regex.match(op2):
            op2_tokenized = self.offset_regex.findall(op2)[0]
            machine_code |= 0b1 << 24

            offset = op2_tokenized[1]
        elif self.pre_regex.match(op2):
            op2_tokenized = self.pre_regex.findall(op2)[0]
            machine_code |= 0b1 << 24
            machine_code |= 0b1 << 22

            offset = op2_tokenized[1]
        elif self.post_regex.match(op2):
            op2_tokenized = self.post_regex.findall(op2)[0]
            machine_code |= 0b1 << 22
        rn = self.register_map[op2_tokenized[0]]
        machine_code |= rn << 17
        offset = op2_tokenized[1]
        if self.immediate_regex.match(offset):
            offset_tokenized = self.immediate_regex.findall(offset)[0]
            if offset_tokenized[0] != "-":
                machine_code |= 0b1 << 23
            machine_code |= (int(offset_tokenized[1]) & 0x1fff)
            return machine_code
        elif self.index_regex.match(offset):
            offset_tokenized = self.index_regex.findall(offset)[0]
            machine_code |= 0b1 << 25
            if offset_tokenized[0] != "-":
                machine_code |= 0b1 << 23
            machine_code |= self.register_map[offset_tokenized[1]]
            return machine_code
        elif self.shifted_index_regex.match(offset):
            offset_tokenized = self.shifted_index_regex.findall(offset)[0]
            machine_code |= 0b1 << 25
            if offset_tokenized[0] != "-":
                machine_code |= 0b1 << 23
            machine_code |= self.register_map[offset_tokenized[1]]
            machine_code |= self.shift_map[offset_tokenized[2]] << 6
            machine_code |= (int(offset_tokenized[3]) & 0b11111) << 8
            return machine_code

    def assemble_alu(self, command, instruction):
        command = command.strip()
        condition = 0b1110
        offset = 0b0

        mov_regex = re.compile("(MOV|MVN)(\w\w)?(S)?\s+(R\d\d?)\s*,\s*(\S.+)\Z")
        test_regex = re.compile("(CMP|CMN|TSQ|TST)(\w\w)?\s+(R\d\d?)\s*,\s*(\S.+)\Z")
        areth_regex = re.compile("(AND|EOR|SUB|RSB|ADD|ADC|SBC|RSC|ORR|BIC)(\w{2})?(S)?\s+(R\d\d?)\s*,\s*(R\d\d?)\s*,\s*(\S.+)\Z")
        machine_code = 0
        machine_code |= instruction["instruction_code"] << 21
        op2 = 0
        if mov_regex.match(command):
            tokenized = mov_regex.findall(command)[0]
            machine_code |= self.condition_code[tokenized[1]] << 28
            if tokenized[2] == "S":
                machine_code |= 0b1 << 20
            machine_code |= self.register_map[tokenized[3]] << 16
            op2 = tokenized[4]
        if test_regex.match(command):
            tokenized = test_regex.findall(command)[0]
            machine_code |= self.condition_code[tokenized[1]] << 28
            machine_code |= self.register_map[tokenized[2]] << 12
            op2 = tokenized[3]
        if areth_regex.match(command):
            tokenized = areth_regex.findall(command)[0]
            machine_code |= self.condition_code[tokenized[1]] << 28
            if tokenized[2] == "S":
                machine_code |= 0b1 << 20
            machine_code |= self.register_map[tokenized[3]] << 16
            machine_code |= self.register_map[tokenized[4]] << 12
            op2 = tokenized[5]

        register_only_regex = re.compile("(R\d\d?)\s*\Z")
        register_shift_regex = re.compile("(R\d\d?)\s*,\s*(\w{3})\s*(\S.*\S)*\Z")
        immediate_only_regex = re.compile("#(\d+)\s*\Z")
        expression = re.compile("#(\d+)\s*#(\d+)\Z")
        register = re.compile("(R\d\d)\s*\Z")

        if register_only_regex.match(op2):
            op2_tokenized = register_only_regex.findall(op2)[0]
            machine_code |= self.register_map[op2_tokenized]
            return machine_code
        if register_shift_regex.match(op2):
            op2_tokenized = register_shift_regex.findall(op2)[0]
            machine_code |= self.register_map[op2_tokenized[0]]
            machine_code |= self.shift_map[op2_tokenized[1]] << 5
            if immediate_only_regex.match(op2_tokenized[2]):
                shift_token = immediate_only_regex.findall(op2_tokenized[2])[0]
                machine_code |= (int(shift_token) & 0b11111) << 7
            if register.match(op2_tokenized[2]):
                shift_token = register_only_regex.findall(op2_tokenized[2])[0]
                machine_code |= self.register_map[shift_token] << 8
                machine_code |= 0b1 << 4
            return machine_code
        if immediate_only_regex.match(op2):
            op2_tokenized = immediate_only_regex.findall(op2)[0]
            machine_code |= int(op2_tokenized)
            machine_code |= 0b1 << 25
            return machine_code
        if expression.match(op2):
            op2_tokenized = expression.findall(op2)[0]
            machine_code |= int(op2_tokenized[0]) & 0xff
            machine_code |= 0b1 << 25
            machine_code |= (int(op2_tokenized[1]) & 0xf) << 8
            return machine_code

    def assemble(self, command):
        instruction = self.instruction_map[command.split()[0][:3]]
        if instruction["instruction_code"] == 0b10:
            return self.assemble_control(command, instruction)
        if instruction["instruction_code"] == 0b01:
            return self.assemble_memory(command, instruction)
        if instruction["instruction_code"] == 0b00:
            return self.assemble_alu(command, instruction)



Assembler("assembly_program.txt")