#include "pch.h"

void Interface::help() {
	std::cout << "Commands: (r)ead, (w)rite, (d)isplay memory, run commands from (f)ile, (e)xecute program, (h)elp" << std::endl;
}

void Interface::poll() {
	std::string filename;
	std::ifstream file;
	std::string line;
	int old_t;
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
		std::cout << "Address to read? ";
		std::cin >> addr;
		std::cout << "Mode? (s)tep or (c)omplete: ";
		std::cin >> mod;

		switch (tolower(mod)) {
		case 's':
			result = f_cpu->read(addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
				std::cout << "Read result: " << std::hex << result << std::endl;
			}
			break;
		case 'c':
			old_t = f_cpu->get_clock();
			result = 0;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				result = f_cpu->read(addr);
			}
			result = f_cpu->read(addr);
			std::cout << "Time taken: " << (f_cpu->get_clock() - old_t) << std::endl;
			std::cout << "Read result: " << std::hex << result << std::endl;
			f_cpu->mem->reset_timer(addr);
			break;
		default:
			std::cout << "Mode not recognized" << std::endl;
		}
		break;
	case 'w':
		uint32_t word;
		std::cout << "Address to write to? ";
		std::cin >> addr;
		std::cout << "Word to write? ";
		std::cin >> word;
		std::cout << "Mode? (s)tep or (c)omplete: ";
		std::cin >> mod;

		switch (tolower(mod)) {
		case 's':
			f_cpu->write(word, addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
			}
			break;
		case 'c':
			old_t = f_cpu->get_clock();
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				f_cpu->write(word, addr);
			}
			f_cpu->write(word, addr);
			f_cpu->mem->reset_timer(addr);
			std::cout << "Time taken: " << (f_cpu->get_clock() - old_t) << std::endl;
			break;
		default:
			std::cout << "Mode not recognized" << std::endl;
		}
		break;
	case 'd':
		int mod;
		cur = f_cpu->mem;
		std::cout << "Level of memory to read? (1 indexed) ";
		std::cin >> mod;
		
		for (int i = 1; i < mod; i++) {
			cur = cur->next_level;
		}
		cur->display();
		break;
	case 'f':
		std::cout << "Please enter filename: ";
		std::cin >> filename;
		file.open(filename);

		while (std::getline(file, line)) {
			std::cout << line << std::endl;
			std::stringstream s(line);
			s >> com;
			switch_cmd(com, s);
		}
		file.close();
		break;
	case 'e':
		execute();
		break;
	default:
		std::cout << "Command not recognized" << std::endl;
	}

}

void Interface::execute() {
	char cmd;
	bool with_pipe, with_cache;
	std::string filename;
	std::ifstream file;
	uint32_t inst;
	std::string line;
	int bp = -1; // break point

	std::cout << "Pipeline? y/n" << std::endl;
	std::cin >> cmd;
	with_pipe = (tolower(cmd) == 'y');
	std::cout << "Cache? y/n" << std::endl;
	std::cin >> cmd;
	with_cache = (tolower(cmd) == 'y');
	std::cout << "Filename? ";
	std::cin >> filename;
	file.open(filename);

	while (!file.eof()) {
		std::cout << "(s)tep, (c)omplete, (b)reakpoint, for (n) cycles, save s(t)ate, (l)oad state, (r)eset state, (v)iew registers, (d)isplay memory" << std::endl;
		std::cin >> cmd;

		switch (tolower(cmd)) {
		case 's':
			std::getline(file, line);
			inst = (uint32_t)std::stoul(line);
			f_cpu->step(with_pipe, with_cache, inst);
			std::cout << "Clock: " << f_cpu->get_clock() << std::endl;
			if (f_cpu->get_pc() == bp)
				std::cout << "Breakpoint reached." << std::endl;
			break;
		case 'c':
			while (std::getline(file, line) && f_cpu->get_pc() != bp) {
				inst = (uint32_t)std::stoul(line);
				f_cpu->step(with_pipe, with_cache, inst);
				std::cout << "Clock: " << f_cpu->get_clock() << std::endl;
			}
			if (f_cpu->get_pc() == bp)
				std::cout << "Breakpoint reached." << std::endl;
			break;
		case 'b':
			std::cout << "Break at which line?" << std::endl;
			std::cin >> bp;
			break;
		case 'n':
			std::cout << "Run for how many cycles?" << std::endl;
			int cycles;
			std::cin >> cycles;
			for (int i = 0; i < cycles && std::getline(file, line) && f_cpu->get_pc() != bp; i++) {
				inst = (uint32_t)std::stoul(line);
				f_cpu->step(with_pipe, with_cache, inst);
				std::cout << "Clock: " << f_cpu->get_clock() << std::endl;
			}
			if (f_cpu->get_pc() == bp)
				std::cout << "Breakpoint reached." << std::endl;
			break;
		case 't':
			if (save_state != 0) {
				std::cout << "Old state will be overwritten. Is this okay? y/n" << std::endl;
				std::cin >> cmd;
				if (tolower(cmd) == 'n') break;
			}
			save_state = f_cpu;
			std::cout << "State saved." << std::endl;
			break;
		case 'l':
			if (save_state != 0) {
				std::cout << "Current state will be overwritten. Is this okay? y/n" << std::endl;
				std::cin >> cmd;
				if (tolower(cmd) == 'n') break;
				f_cpu = save_state;
			} else {
				std::cout << "Nothing to load." << std::endl;
			}
			break;
		case 'r':
			std::cout << "This will reset the state of the processor. Is this okay? y/n" << std::endl;
			std::cin >> cmd;
			if (tolower(cmd) == 'n') break;
			f_cpu = new CPU(f_cpu->get_word_size());
			break;
		case 'v':
			f_cpu->display_registers();
			break;
		case 'd':
			std::cout << "What level of memory? (1 indexed)" << std::endl;
			std::cin >> cmd;
			std::cout << "Display how many sets?" << std::endl;
			int size;
			std::cin >> size;
			std::cout << "Starting at what address?" << std::endl;
			uint32_t addr;
			std::cin >> addr;
			f_cpu->display_mem(addr, size, cmd);
			break;
		default:
			std::cout << "Command not recognized." << std::endl;
			break;
		}
	}

	file.close();
}

void Interface::switch_cmd(char cmd, std::stringstream& file) {
	std::string filename;
	std::ifstream e_file;
	std::string line;
	char com, mod;
	uint32_t addr;
	Memory* cur = 0;

	switch (tolower(cmd)) {
	case 'h':
		help();
		break;
	case 'r':
		file >> addr;
		file >> mod;
		switch_mod(mod, cmd, addr, 0, file);
		break;
	case 'w':
		uint32_t word;
		file >> addr;
		file >> word;
		file >> mod;
		switch_mod(mod, cmd, addr, word, file);
		break;
	case 'd':
		int mod;
		cur = f_cpu->mem;
		file >> mod;

		for (int i = 1; i < mod; i++) {
			cur = cur->next_level;
		}
		cur->display();
		break;
	case 'f':
		file >> filename;
		e_file.open(filename);

		while (std::getline(e_file, line)) {
			std::cout << line << std::endl;
			std::stringstream s(line);
			s >> com;
			switch_cmd(com, s);
		}
		break;
	default:
		std::cout << "Command not recognized" << std::endl;
	}
}

void Interface::switch_mod(char mod, char cmd, uint32_t addr, uint32_t word, std::stringstream& file) {
	uint32_t old_t, result;

	switch (tolower(cmd)) {
	case 'r':
		switch (tolower(mod)) {
		case 's':
			result = f_cpu->read(addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
				std::cout << "Read result: " << std::hex << result << std::endl;
			}
			break;
		case 'c':
			old_t = f_cpu->get_clock();
			result = 0;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				result = f_cpu->read(addr);
			}
			result = f_cpu->read(addr);
			std::cout << "Time taken: " << (f_cpu->get_clock() - old_t) << std::endl;
			std::cout << "Read result: " << std::hex << result << std::endl;
			f_cpu->mem->reset_timer(addr);
			break;
		}
		break;
	case 'w':
		switch (tolower(mod)) {
		case 's':
			f_cpu->write(word, addr);
			if (f_cpu->mem->query_timer(addr) == f_cpu->mem->get_latency()) {
				f_cpu->mem->reset_timer(addr);
			}
			break;
		case 'c':
			old_t = f_cpu->get_clock();
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				f_cpu->write(word, addr);
			}
			f_cpu->write(word, addr);
			f_cpu->mem->reset_timer(addr);
			std::cout << "Time taken: " << (f_cpu->get_clock() - old_t) << std::endl;
			break;
		}
		break;
	}
}