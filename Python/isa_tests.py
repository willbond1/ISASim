import struct
import unittest
from cpu import CPU
from mem import Cache, RAM

empty_stage = word_mask = int(('FF' * CPU.word_size), 16)

class TestISA(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestISA, self).__init__(*args, **kwargs)
        self.f_cpu = CPU()
        self.l1 = Cache(5, 2, 32, 8)
        self.mem = RAM(10, 128, 4)

        self.l1.set_next_level(self.mem)
        self.l1.set_CPU(self.f_cpu)
        self.mem.set_CPU(self.f_cpu)
        self.f_cpu.set_memory(self.l1)
    
    # basic write/test
    def test_write_read(self):
        word = 27
        self.f_cpu.write(0x00, word)
        self.assertEqual(word, self.f_cpu.read(0x00))

        word = 126
        self.f_cpu.write(0x16, word)
        self.assertEqual(word, self.f_cpu.read(0x16))
    
    def test_fetch(self):
        inst_1 = 4783
        self.f_cpu.write(0x10, inst_1)
        inst_2 = 90321
        self.f_cpu.write(0x10 + 4, inst_2)
        self.f_cpu.registers[15] = 0x10

        self.assertEqual(inst_1, self.f_cpu.read(0x10))
        self.assertEqual(inst_2, self.f_cpu.read(0x14))

        fetch_1 = self.f_cpu.fetch(self.f_cpu.memory)
        while not fetch_1:
            fetch_1 = self.f_cpu.fetch(self.f_cpu.memory)
        
        fetch_2 = self.f_cpu.fetch(self.f_cpu.memory)
        while not fetch_2:
            fetch_2 = self.f_cpu.fetch(self.f_cpu.memory)
        
        self.assertEqual(inst_1, fetch_1)
        self.assertEqual(inst_2, fetch_2)

    def test_decode(self):
        inst = 0b11100100001010000000000000011110
        decoded_inst = (0b1110, 0b01, 0b0, 0b0, 0b0, 0b0, 0b1, 0b0100, 0b0000, 0b0000000011110)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b11100100001010000010000000100110
        decoded_inst = (0b1110, 0b01, 0b0, 0b0, 0b0, 0b0, 0b1, 0b0100, 0b0001, 0b0000000100110)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b11100000100000100001000000000000
        decoded_inst = (0b1110, 00, 0, 0b0100, 0, 0b0010, 0b0001, 00000, 00, 0, 0000)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b11100100000010000100000000011110
        decoded_inst = (0b1110, 0b01, 0, 0, 0, 0, 0, 0b0100, 0b0010, 0b0000000011110)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b11100011010101000010000000000110
        decoded_inst = (0b1110, 00, 1, 0b1010, 1, 0b0100, 0b0010, 0000, 0b00000110)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b00001010111111111111111111111011
        decoded_inst = (0000, 0b10, 1, 0, 0b111111111111111111111011)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())

        inst = 0b00001100000000000000000000000000
        decoded_inst = (0, 0b11)
        self.f_cpu.fetch_stage = inst
        self.assertEqual(decoded_inst, self.f_cpu.decode())
    
    def test_execute(self):
        # memory instructions
        self.f_cpu.registers[6] = 0x20
        self.f_cpu.registers[10] = 56
        inst = 0b11100100100011010100000000000010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), (0x20, 0))

        inst = 0b11100101000011010100000000000010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), (0x1E, 0))

        inst = 0b11100101100011010100000000000010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), (0x22, 0))

        inst = 0b11100101110011010100000000000010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), (0x22, 0x22))
        self.assertEqual(self.f_cpu.forward_register[6], 0x22)

        # control instructions
        self.f_cpu.registers[15] = 0x10
        inst = 0b11101011000000000000000000000010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), 0x18)
        self.assertEqual(self.f_cpu.fetch_stage, empty_stage)
        self.assertEqual(self.f_cpu.decode_stage, empty_stage)
        self.assertEqual(self.f_cpu.registers[14], 0xC)

        self.f_cpu.Z = True
        self.f_cpu.registers[15] = 0x18
        inst = 0b00001010111111111111111111111010
        self.f_cpu.fetch_stage = inst
        self.f_cpu.decode_stage = self.f_cpu.decode()
        self.assertEqual(self.f_cpu.execute(), 0x0)
        self.assertEqual(self.f_cpu.registers[14], 0xC)

if __name__ == '__main__':
    unittest.main()