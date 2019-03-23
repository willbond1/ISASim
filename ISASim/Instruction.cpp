#include "pch.h"
#include "CPU.h"




void CPU::Instruction::decode()
{
	condition_code = (machine_code & 0xf0000000) >> 28;
	instruction_code = (machine_code & 0x0c000000) >> 26;
	switch (instruction_code)
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

void CPU::Instruction::decode_ALU()
{
	Rd = &(instruction_CPU->registers[(machine_code & 0xF0000) >> 16]);
	Op1 = instruction_CPU->registers[(machine_code & 0xF000) >> 12];
	bool S = machine_code & 0x100000;
	bool I = machine_code & 0x2000000;
	
}

void CPU::Instruction::decode_Memory()
{
}

void CPU::Instruction::decode_Control()
{
}

void CPU::Instruction::no_op()
{
}

bool CPU::Instruction::condition_valid(uint8_t code)
{
	switch (code)
	{
	case 0:
		return instruction_CPU->Z_flag;
	case 1:
		return !instruction_CPU->Z_flag;
	case 2:
		return instruction_CPU->C_flag;
	case 3:
		return !instruction_CPU->C_flag;
	case 4:
		return instruction_CPU->N_flag;
	case 5:
		return !instruction_CPU->N_flag;
	case 6:
		return instruction_CPU->V_flag;
	case 7:
		return !instruction_CPU->V_flag;
	case 8:
		return instruction_CPU->C_flag && !instruction_CPU->Z_flag;
	case 9:
		return !instruction_CPU->C_flag || instruction_CPU->Z_flag;
	case 10:
		return instruction_CPU->N_flag == instruction_CPU->V_flag;
	case 11:
		return instruction_CPU->N_flag != instruction_CPU->V_flag;
	case 12:
		return !instruction_CPU->Z_flag && (instruction_CPU->N_flag == instruction_CPU->V_flag);
	case 13:
		return instruction_CPU->Z_flag || (instruction_CPU->N_flag != instruction_CPU->V_flag);
	case 14:
		return true;
	case 15:
		return false;
	default:
		break;
	}
}
