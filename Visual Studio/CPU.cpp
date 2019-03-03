#include "pch.h"
#include "CPU.h"

uint32_t CPU::read(long addr) {
	return mem->read(mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
}

void CPU::write(uint32_t word, long addr) {
	mem->write(word, mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
}