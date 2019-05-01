import isasim as sim
import cpu

breaks = set()

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
        mem_level = sim.processor.memory
        while mem_level.next_level is not None:
            mem_level = mem_level.next_level
        for line in (open(input_string[1], "r")):
            for num, line_num in enumerate(line.split()):
                print(line_num, num)
                mem_level.write_complete(num*4, int(line_num).to_bytes(4, byteorder="big"))
    # LOAD MEMORY
    elif input_string[0] == "LM":
        sim.processor.write(int(input_string[2]), int(input_string[1]))
    # READ MEMORY
    elif input_string[0] == "RM":
        print(sim.processor.read(int(input_string[1])))
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
        breaks.add(int(input_string[1]))
    elif input_string[0] == "CM":
        breaks.add(int(input_string[1]))
        while sim.processor.registers[15] not in breaks:
            sim.processor.step(True, True)
    elif input_string[0] == "DB":
        print([i for i in breaks])
    elif input_string[0] == "RB":
        breaks.discard(int(input_string[1]))

    else:
        for line in help:
            print(line)
    input_string = input()