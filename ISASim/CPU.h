#pragma once
#include <vector>
#include "Memory.h"
#include "Interface.h"

constexpr auto SP = 13;
constexpr auto LR = 14;
constexpr auto PC = 15;

typedef uint32_t Register;

class CPU {
private:
	friend class Interface;
	friend class Pipeline;
	int word_size;
	Memory *mem = 0;
	Register registers[16];
	bool N_flag = false;
	bool Z_flag = false;
	bool C_flag = false;
	bool V_flag = false;


	struct Instruction
	{
		uint32_t machine_code;
		uint8_t condition_code;
		uint8_t instruction_code;
		uint8_t opcode;
		uint8_t rd_number;
		uint8_t op1_number;
		uint8_t op2_number;
		uint8_t off_number;
		uint8_t off_shift_number;
		uint8_t off_shift_type;
		bool write_back;
		bool post_index;
		bool add_sub_offset;
		bool load_write;
		bool offset_register;
		bool offset_shift_register
	};

	class Pipeline
	{
	private:
		CPU *CPU;
		Instruction fetch_ins;
		Instruction decode_ins;
		Instruction execute_ins;
		Instruction memory_ins;
		Instruction writeback_ins;

	public:
		Pipeline();
		~Pipeline();
		void pipelineController();
		void flushPipeline();
		void decode();
		void decode_ALU();
		void decode_Memory();
		void decode_Control();
		void no_op();
		bool condition_valid(uint8_t code);
	};

	


public:
	CPU(int l_word_size) : word_size(l_word_size) {}
	int get_word_size() { return word_size; }
	void attach_memory(Memory *l_mem) { mem = l_mem; }

	uint32_t read(uint32_t addr);
	void write(uint32_t word, uint32_t addr);
};