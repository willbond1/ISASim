#pragma once
#include "CPU.h"

class Interface
{
private:
	CPU* f_cpu;

public:
	Interface(CPU* l_cpu): f_cpu(l_cpu) {}

	void help();
	void poll();
};

