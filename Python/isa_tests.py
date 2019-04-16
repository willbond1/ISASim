import struct
import unittest
from cpu import CPU
from mem import Cache, RAM

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
        word_bytes = word.to_bytes(4, byteorder='big')
        self.f_cpu.write(0x00, word_bytes)
        self.assertEqual(word_bytes, self.f_cpu.read(0x00))

if __name__ == '__main__':
    unittest.main()