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

void CPU::display_mem(uint32_t addr, uint32_t size) {

}