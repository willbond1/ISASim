// ISASim.cpp: main driver

#include "pch.h"
#include <iostream>
#include "CPU.h"
#include "Memory.h"

int main() {
	std::cout << "Welcome to the simulation." << std::endl;

	CPU my_CPU(32);
	std::cout << "CPU created with word size: " << 32 << std::endl << std::endl;

	Memory L1(5, 8, 32 * 1000, 64, 32);
	std::cout << "L1 Cache:" << std::endl;
	L1.print();

	my_CPU.attach_memory(&L1);
	L1.display();

	for (int i = 0; i < 6; i++) {
		my_CPU.write(42, 64);
	}

	for (int i = 0; i < 5; i++) {
		my_CPU.read(64);
	}

	std::cout << my_CPU.read(64) << std::endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu