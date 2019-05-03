import isasim as sim
import cpu

breaks = set()

help = ["FL filename: load a file into memory", "LM word addr: load value into memory",
        "RM addr: read value from memory", "DM mem_level start size: view memory level",
        "ST [C/P] [C/P]: Step instruction with cache and/or/nor pipe", "DC: Display CPU", "BR addr: add a breakpoint at addr",
        "CM addr [C/P] [C/P]: run until addr with cache and/or/nor pipe", "HELP: display help again", "EXIT: exit"]
for line in help:
    print(line)
input_string = input('> ')
while input_string != "exit":
    input_string = input_string.upper().split()

    if not input_string:
        pass
    # FILE LOAD
    elif input_string[0] == "FL":
        mem_level = sim.processor.memory
        while mem_level.next_level is not None:
            mem_level = mem_level.next_level
        for num, line in enumerate(open(input_string[1], "r")):
            print(num*4, line)
            mem_level.write_complete(num*4, int(line).to_bytes(4, byteorder="big"))
            sim.processor.registers[13] = (num+1)*4
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
        cache = False
        pipe = False
        if len(input_string) > 1:
            if input_string[1] == "C":
                cache = True
            elif input_string[1] == "P":
                pipe = True
        if len(input_string) > 2:
            if input_string[2] == "C":
                cache = True
            elif input_string[2] == "P":
                pipe = True
        sim.processor.step(cache, pipe)
    elif input_string[0] == "DC":
        sim.processor.display_cpu()
    elif input_string[0] == "BR":
        breaks.add(int(input_string[1]))
    elif input_string[0] == "CM":
        curr_pc = sim.processor.registers[15]
        cache = False
        pipe = False
        if len(input_string) > 2:
            if input_string[2] == "C":
                cache = True
            elif input_string[2] == "P":
                pipe = True
        if len(input_string) > 3:
            if input_string[3] == "C":
                cache = True
            elif input_string[3] == "P":
                pipe = True
        while curr_pc == sim.processor.registers[15]:
            sim.processor.step(cache, pipe)
        while sim.processor.registers[15] not in breaks and sim.processor.registers[15] != int(input_string[1]):
            sim.processor.step(cache, pipe)
    elif input_string[0] == "DB":
        print([i for i in breaks])
    elif input_string[0] == "RB":
        breaks.discard(int(input_string[1]))

    else:
        for line in help:
            print(line)
    input_string = input('> ')