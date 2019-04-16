from .mem import Cache, RAM
from .cpu import CPU

class Assembler:

    def __init__(self, file):
        self.symbol_table = {}
        self.file = file