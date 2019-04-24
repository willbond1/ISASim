from mem import Cache, RAM
from cpu import CPU


processor = CPU()
l1_cache = Cache(4, 2, 64, 32)
ram = RAM(100, 8*32, 32)

processor.set_memory(l1_cache)
l1_cache.set_next_level(ram)
l1_cache.set_CPU(processor)
ram.set_CPU(processor)

# print('Processor set up successfully')
#
# if __name__ == '__main__':
#     main()