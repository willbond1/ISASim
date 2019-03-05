#include "pch.h"

uint32_t CPU::read(uint32_t addr) {
	return mem->read(addr);
}

void CPU::write(uint32_t word, uint32_t addr) {
	mem->write(word, addr);
}