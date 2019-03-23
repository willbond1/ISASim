#pragma once
#include <vector>
#include "Memory.h"
#include "Interface.h"

typedef uint32_t Register;

class CPU {
private:
	friend class Interface;
	friend class Instruction;
	friend class Pipeline;
	int word_size;
	Memory *mem = 0;
	Register registers[16];
	bool N_flag = false;
	bool Z_flag = false;
	bool C_flag = false;
	bool V_flag = false;

	class Instruction
	{
	private:
		CPU *instruction_CPU;
		uint32_t machine_code;
		uint8_t condition_code;
		uint8_t instruction_code;
		Register* Rd;
		Register Op1;
		uint32_t Op2;
	public:
		Instruction(uint32_t machine_code, CPU* CPU) : machine_code(machine_code), instruction_CPU(CPU) {};
		~Instruction();
		void decode();
		void decode_ALU();
		void decode_Memory();
		void decode_Control();
		void no_op();
		bool condition_valid(uint8_t code);


	};


	class Pipeline
	{
	private:
		Instruction fetch_inst;
		Instruction decode_inst;
		Instruction execute_inst;
		Instruction memory_inst;
		Instruction writeback_inst;

	public:
		Pipeline();
		~Pipeline();
		void pipelineController();
		void flushPipeline();

	};

	


public:
	CPU(int l_word_size) : word_size(l_word_size) {}
	int get_word_size() { return word_size; }
	void attach_memory(Memory *l_mem) { mem = l_mem; }

	uint32_t read(uint32_t addr);
	void write(uint32_t word, uint32_t addr);
};