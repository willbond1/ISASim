#include "pch.h"
#include "CPU.h"

uint32_t CPU::read(uint32_t addr) {
	clock++;
	if (addr % 8 != 0) { // address is not byte aligned
		std::cout << "Error: address is not byte aligned" << std::endl;
		return 0;
	}

	if (mem->query_timer(addr) == mem->get_latency()) {
		mem->reset_timer(addr);
		return mem->read(mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
	}

	mem->increment_timer(addr);
	std::cout << "Waiting..." << std::endl;
	return 0;
}

void CPU::write(uint32_t word, uint32_t addr) {
	clock++;
	if (addr % 8 != 0) { // address is not byte aligned
		std::cout << "Error: address is not byte aligned" << std::endl;
		return;
	}

	if (mem->query_timer(addr) == mem->get_latency()) {
		mem->reset_timer(addr);
		mem->write(word, mem->decode_index(addr), mem->decode_tag(addr), mem->decode_offset(addr));
		std::cout << "Write successful" << std::endl;
	} else {
		mem->increment_timer(addr);
		std::cout << "Waiting..." << std::endl;
	}
}