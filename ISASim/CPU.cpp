#include "pch.h"

uint32_t CPU::read(uint32_t addr) {
	return mem->read(addr);
}

void CPU::write(uint32_t word, uint32_t addr) {
	mem->write(word, addr);
}

void CPU::display_registers() {
	for (int i = 0; i < 13; i++) {
		std::cout << "Contents of register " << i << ": " << registers[i] << std::endl;
	}
	std::cout << "Contents of PC: " << registers[PC] << std::endl
		<< "Contents of LR: " << registers[LR] << std::endl
		<< "Contents of SP: " << registers[SP] << std::endl << std::endl
		<< "N flag: " << N_flag << std::endl
		<< "Z flag: " << Z_flag << std::endl
		<< "C flag: " << C_flag << std::endl
		<< "V flag: " << V_flag << std::endl << std::endl;

	pipe.display_contents();
}

void CPU::display_mem(uint32_t addr, uint32_t sets, int lvl) {
	Memory *cur_mem = mem;
	for (int i = 1; i < lvl && cur_mem->get_next_level() != 0; i++) {
		cur_mem = cur_mem->get_next_level();
	}
	cur_mem->display_memory(addr, sets);
}

void CPU::deep_copy(CPU *l_cpu) {
	l_cpu->attach_memory(mem);
	l_cpu->word_size = this->word_size;
	for (int i = 0; i < 16; i++) {
		l_cpu->registers[i] = this->registers[i];
	}

	l_cpu->N_flag = this->N_flag;
	l_cpu->Z_flag = this->Z_flag;
	l_cpu->C_flag = this->C_flag;
	l_cpu->V_flag = this->V_flag;

	l_cpu->clock = this->clock;
	pipe.deep_copy(&(l_cpu->pipe));
}