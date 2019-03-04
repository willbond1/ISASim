#include "pch.h"

void Interface::help() {
	std::cout << "To use: [command] [mode] [address?] [word?]\n" << std::endl
		<< "command: r = read, w = write, d = display memory, h to view help again" << std::endl
		<< "mode: (r/w) s = step, c = run to completion; (d) n = memory level depth to access" << std::endl;
}

void Interface::poll() {
	char com, mod;
	uint32_t addr;
	Memory* cur = 0;
	std::cin >> com;

	switch (tolower(com)) {
	case 'h':
		help();
		break;
	case 'r':
		uint32_t result;
		std::cin >> mod;
		std::cin >> addr;

		switch (tolower(mod)) {
		case 's':
			result = f_cpu->read(addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
				std::cout << "Read result: " << std::hex << result << std::endl;
			}
			break;
		case 'c':
			result = 0;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				result = f_cpu->read(addr);
			}
			result = f_cpu->read(addr);
			std::cout << "Read result: " << std::hex << result << std::endl;
			f_cpu->mem->reset_timer(addr);
			break;
		default:
			std::cout << "Mode not recognized" << std::endl;
		}
		break;
	case 'w':
		uint32_t word;
		std::cin >> mod;
		std::cin >> addr;
		std::cin >> word;

		switch (tolower(mod)) {
		case 's':
			f_cpu->write(word, addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
			}
			break;
		case 'c':
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				f_cpu->write(word, addr);
			}
			f_cpu->write(word, addr);
			f_cpu->mem->reset_timer(addr);
			break;
		default:
			std::cout << "Mode not recognized" << std::endl;
		}
		break;
	case 'd':
		int mod;
		cur = f_cpu->mem;
		std::cin >> mod;
		
		for (int i = 1; i < mod; i++) {
			cur = cur->next_level;
		}
		cur->display();
		break;
	default:
		std::cout << "Command not recognized" << std::endl;
		return;
	}

}