from mem import Cache, RAM

class CPU:
    SP = 13
    LR = 14
    PC = 15
    word_size = 4 # word size in bytes
    registers = [0] * 16
    N = False
    Z = False
    C = False
    V = False
    clock = 0

    def __init__(self):
        pass