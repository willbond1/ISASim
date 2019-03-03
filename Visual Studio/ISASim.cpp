// ISASim.cpp: main driver

#include "pch.h"
#include <iostream>

int main() {
	std::cout << "Welcome to the simulation." << std::endl;

	CPU my_CPU(32);
	std::cout << "CPU created with word size: " << 32 << std::endl << std::endl;
	
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu