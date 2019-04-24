import isasim as sim
import cpu

breaks = []

help = ["FL filename: load a file into memory", "LM addr: load value into memory",
        "RM word addr: read value from memory", "DM mem_level start size: view memory level", "ST: Step instruction", "DC: Display CPU", "BR addr: add a breakpoint at addr", "CM addr: run until addr"]
for line in help:
    print(line)
input_string = input()
while input_string != "exit":
    input_string = input_string.upper().split()

    if not input_string:
        pass
    # FILE LOAD
    elif input_string[0] == "FL":
        final=0
        for num, line in enumerate(open(input_string[1], "r")):
            sim.processor.write(num, int(line).to_bytes(cpu.CPU.word_size, byteorder="big"))
            final = num+1
        sim.processor.write(final, 0xfffffff0.to_bytes(cpu.CPU.word_size, byteorder="big"))
    # LOAD MEMORY
    elif input_string[0] == "LM":
        sim.processor.write(int(input_string[2]), (int(input_string[1]).to_bytes(cpu.CPU.word_size, byteorder="big")))
    # READ MEMORY
    elif input_string[0] == "RM":
        print(int.from_bytes(sim.processor.read(int(input_string[1])), byteorder="big"))
    # DISPLAY MEMORY
    elif input_string[0] == "DM":
        mem_level = sim.processor.memory
        for i in range(int(input_string[1])):
            if mem_level.next_level is not None:
                mem_level = mem_level.next_level
        mem_level.display(int(input_string[2]), int(input_string[3]))
    elif input_string[0] == "ST":
        sim.processor.step(True, True)
    elif input_string[0] == "DC":
        sim.processor.display_cpu()
    elif input_string[0] == "BR":
        breaks += [int(input_string[1])]
    elif input_string[0] == "CM":
        breaks += [int(input_string[1])]
        while sim.processor.registers[15] not in breaks:
            sim.processor.step(True, True)
    else:
        for line in help:
            print(line)
    input_string = input()
