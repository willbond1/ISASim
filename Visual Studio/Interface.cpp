#include "pch.h"

void Interface::help() {
	std::cout << "To use: [command] [mode]\n" << std::endl
		<< "command: r = read, w = write, h to view help again" << std::endl
		<< "mode: s = step, c = run to completion" << std::endl;
}

void Interface::poll() {
	
}