#include "pch.h"
#include "CPU.h"


CPU::Pipeline::Pipeline()
{
}


CPU::Pipeline::~Pipeline()
{
}

void CPU::Pipeline::pipelineController()
{
}

void CPU::Pipeline::flushPipeline()
{
}


void CPU::Pipeline::decode()
{
	decode_ins.condition_code = (decode_ins.machine_code & 0xf0000000) >> 28;
	decode_ins.instruction_code = (decode_ins.machine_code & 0x0c000000) >> 26;
	switch (decode_ins.instruction_code)
	{
	case 0:
		decode_ALU();
		break;
	case 1:
		decode_Memory();
		break;
	case 2:
		decode_Control();
		break;
	case 3:
		no_op();
		break;
	default:
		std::cout << "Problem in IC decode";
		break;
	}

}

/*		uint32_t machine_code;
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
*/

void CPU::Pipeline::decode_ALU()
{
	decode_ins.post_index = false;
	decode_ins.add_sub_offset = false;
	decode_ins.write_back = true;
	decode_ins.load_write = true;
	decode_ins.rd_number = (decode_ins.machine_code & 0xF0000) >> 16;
	decode_ins.op1_number = (decode_ins.machine_code & 0xF000) >> 12;
	bool S = decode_ins.machine_code & 0x100000;
	bool I = decode_ins.machine_code & 0x2000000;
	if (I) {
		decode_ins.offset_register = true;
		decode_ins.op2_number
	}
	else {
		decode_ins.offset_register = false;
	}
}

void CPU::Pipeline::decode_Memory()
{
}

void CPU::Pipeline::decode_Control()
{
}

void CPU::Pipeline::no_op()
{
}

bool CPU::Pipeline::condition_valid(uint8_t code)
{
	switch (code)
	{
	case 0:
		return CPU->Z_flag;
	case 1:
		return !CPU->Z_flag;
	case 2:
		return CPU->C_flag;
	case 3:
		return !CPU->C_flag;
	case 4:
		return CPU->N_flag;
	case 5:
		return !CPU->N_flag;
	case 6:
		return CPU->V_flag;
	case 7:
		return !CPU->V_flag;
	case 8:
		return CPU->C_flag && !CPU->Z_flag;
	case 9:
		return !CPU->C_flag || CPU->Z_flag;
	case 10:
		return CPU->N_flag == CPU->V_flag;
	case 11:
		return CPU->N_flag != CPU->V_flag;
	case 12:
		return !CPU->Z_flag && (CPU->N_flag == CPU->V_flag);
	case 13:
		return CPU->Z_flag || (CPU->N_flag != CPU->V_flag);
	case 14:
		return true;
	case 15:
		return false;
	default:
		break;
	}
}
