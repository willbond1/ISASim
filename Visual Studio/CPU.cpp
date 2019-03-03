#include "pch.h"
#include "CPU.h"

uint32_t CPU::read(long addr) {
	if (mem->query_timer(addr) == mem->get_latency()) {
		mem->reset_timer(addr);
		return mem->read(mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
	}

	mem->increment_timer(addr);
	std::cout << "Waiting..." << std::endl;
	return 0;
}

void CPU::write(uint32_t word, long addr) {
	if (mem->query_timer(addr) == mem->get_latency()) {
		mem->reset_timer(addr);
		mem->write(word, mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
	} else {
		mem->increment_timer(addr);
		std::cout << "Waiting..." << std::endl;
	}
}