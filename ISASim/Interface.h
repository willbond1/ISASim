#pragma once
#include "CPU.h"

class Interface
{
private:
	CPU* f_cpu;
	CPU* save_state = 0;
	void switch_cmd(char cmd, std::stringstream& file);
	void switch_mod(char mod, char cmd, uint32_t addr, uint32_t word, std::stringstream& file);
	void execute();

public:
	Interface(CPU* l_cpu): f_cpu(l_cpu) {}

	void help();
	void poll();
};

