from mem import Memory

SP = 13
LR = 14
PC = 15

class CPU:
    word_size = 32 # word size in bits
    registers = [0] * 16
    N = False
    Z = False
    C = False
    V = False
    clock = 0

    def __init__(self):
        pass