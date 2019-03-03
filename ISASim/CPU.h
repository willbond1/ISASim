#pragma once
#include <vector>
#include "Memory.h"

class CPU {
private:
	int clock = 0;
	int word_size;
	Memory *mem = 0;

public:
	CPU(int l_word_size): word_size(l_word_size) {}
	int get_word_size() { return word_size; }
	void attach_memory(Memory *l_mem) { mem = l_mem; }

	uint32_t read(uint32_t addr);
	void write(uint32_t word, uint32_t addr);
};