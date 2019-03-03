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
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu