from mem import Cache, RAM
from cpu import CPU

def main():
    processor = CPU()
    l1_cache = Cache(4, 8, 32000, 64)
    ram = RAM(100, 1000000000, 64)

    processor.mem = l1_cache
    l1_cache.next_level = ram
    l1_cache.f_cpu = processor
    ram.f_cpu = processor

if __name__ == '__main__':
    main()