import isasim as sim
import cpu
sim.ram.print_stats()
print(sim.ram.decode_offset(65))
print(sim.ram.decode_index(65))
print(sim.ram.decode_tag(65))
help = "FL filename: load a file into memory", "LM loc value: load value into memory", "RM #: read value from memory", "DM #: view memory level", "S: Step instruction",
input_string = input()
while input_string != "exit":
    input_string = input_string.upper().split()
    if input_string[0] == "FL":
        for num, line in enumerate(open(input_string[1], "r")):
            sim.processor.write(num, int(line))
    elif input_string[0] == "LM":
        sim.processor.write(int(input_string[2]), (int(input_string[1]).to_bytes(cpu.CPU.word_size, byteorder="little")))
    elif input_string[0] == "RM":
        print(int.from_bytes(sim.processor.read(int(input_string[1])), byteorder="little"))
    elif input_string[0] == "DM":
        mem_level = sim.processor.memory
        for i in range(int(input_string[1])):
            if mem_level.next_level is not None:
                mem_level = mem_level.next_level
        mem_level.display(int(input_string[2]), int(input_string[3]))

    else:
        for line in help:
            print(line)
    input_string = input()
