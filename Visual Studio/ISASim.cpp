// ISASim.cpp: main driver

#include "pch.h"
#include <iostream>

int main() {
	std::cout << "Welcome to the simulation." << std::endl;

	CPU my_CPU(32);
	Interface interf(&my_CPU);
	Memory RAM(1, 1, 32 * 1000, 64, 32, true);
	Memory L1(5, 8, 8 * 1000, 64, 32, false);
	L1.attach_memory(&RAM);
	my_CPU.attach_memory(&L1);

	interf.help();
	while (true) {
		interf.poll();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu