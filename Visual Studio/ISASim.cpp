// ISASim.cpp: main driver

#include "pch.h"
#include <iostream>

int main() {
	std::cout << "Welcome to the simulation." << std::endl;

	CPU my_CPU(32);
	Interface interf(&my_CPU);
	Memory RAM(10, 1, 320, 8, 32, true);
	Memory L1(5, 2, 16, 8, 32, false);
	L1.attach_memory(&RAM);
	my_CPU.attach_memory(&L1);
	std::cout << "RAM INFO:" << std::endl;
	RAM.print();
	std::cout << "L1 INFO:" << std::endl;
	L1.print();
	std::cout << "----------------------------------" << std::endl;
	interf.help();
	while (true) {
		interf.poll();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu