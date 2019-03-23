#include "pch.h"

void Interface::help() {
	std::cout << "Commands: (r)ead, (w)rite, (d)isplay, run commands from (f)ile, (h)elp" << std::endl;
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
			old_t = clock;
			result = 0;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				result = f_cpu->read(addr);
			}
			result = f_cpu->read(addr);
			std::cout << "Time taken: " << (clock - old_t) << std::endl;
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
			old_t = clock;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				f_cpu->write(word, addr);
			}
			f_cpu->write(word, addr);
			f_cpu->mem->reset_timer(addr);
			std::cout << "Time taken: " << (clock - old_t) << std::endl;
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
		break;
	default:
		std::cout << "Command not recognized" << std::endl;
	}
	if (std::cin.fail())
	{
		std::cin.clear();
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
	if (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
			old_t = clock;
			result = 0;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				result = f_cpu->read(addr);
			}
			result = f_cpu->read(addr);
			std::cout << "Time taken: " << (clock - old_t) << std::endl;
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
			old_t = clock;
			while (f_cpu->mem->query_timer(addr) < f_cpu->mem->get_latency()) {
				f_cpu->write(word, addr);
			}
			f_cpu->write(word, addr);
			f_cpu->mem->reset_timer(addr);
			std::cout << "Time taken: " << (clock - old_t) << std::endl;
			break;
		}
		break;
	}
}