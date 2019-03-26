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


bool CPU::Pipeline::decode()
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

	decode_ins.decoded = true;
}

/*		\uint32_t machine_code;
		\uint8_t condition_code;
		\uint8_t instruction_code;
		\uint8_t opcode;
		\uint8_t rd_number;
		\uint8_t rn_number;
		uint8_t rm_number;
		uint8_t rs_number;
		uint8_t off_shift_type;
		bool write_back;
		bool post_index;
		bool add_sub_offset;
		bool load_write;
		bool rm_register_used;
		bool rs_register_used;
*/

void CPU::Pipeline::decode_ALU()
{
	decode_ins.write_rn = false;
	//Always post-index
	decode_ins.pre_index = false;
	//Always adding signed offset
	decode_ins.add_sub_offset = true;
	//Opcode
	decode_ins.opcode = (decode_ins.machine_code & 0x1e00000) >> 21;
	//Whether writes to register
	decode_ins.write_rd = !(decode_ins.opcode >= 8 && decode_ins.opcode <= 11);
	//Rd
	decode_ins.rd_number = (decode_ins.machine_code & 0xF0000) >> 16;
	//Rn (OP1)
	decode_ins.rn_number = (decode_ins.machine_code & 0xF000) >> 12;
	//S
	decode_ins.update_status = decode_ins.machine_code & 0x100000;
	//I
	decode_ins.rm_register_used = decode_ins.machine_code & 0x2000000;

	if (decode_ins.rm_register_used) {
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
		decode_ins.rs_register_used = decode_ins.machine_code & 0x10;

		if (decode_ins.rs_register_used)
			decode_ins.rs_number = (decode_ins.machine_code & 0xf00) >> 8;
		else
			decode_ins.rs_register_used = false;

		decode_ins.off_shift_type = (decode_ins.machine_code & 0x60) >> 5;
	}
	else {
		decode_ins.rs_register_used = false;
	}
}

void CPU::Pipeline::decode_Memory()
{
	//P
	decode_ins.pre_index = decode_ins.machine_code & 0x1000000;
	//U
	decode_ins.add_sub_offset = decode_ins.machine_code & 0x800000;
	//W
	decode_ins.write_rn = decode_ins.machine_code & 0x400000;
	//L
	decode_ins.opcode = decode_ins.machine_code & 0x200000;
	decode_ins.write_rd = decode_ins.opcode;
	//Rd
	decode_ins.rd_number = (decode_ins.machine_code & 0x1E000) >> 13;
	//Rn (OP1)
	decode_ins.rn_number = (decode_ins.machine_code & 0x1E0000) >> 17;
	//I
	decode_ins.rm_register_used = decode_ins.machine_code & 0x2000000;
	if (decode_ins.rm_register_used) {
		//Rm
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
		//Shift type
		decode_ins.off_shift_type = (decode_ins.machine_code & 0xc0) >> 5;
	}
	else {
		decode_ins.offset_amount = decode_ins.machine_code & 0xfff;
	}
	//Always Immediate Shift Amount
	decode_ins.rs_register_used = false;
}

void CPU::Pipeline::decode_Control()
{
	decode_ins.rm_register_used = decode_ins.machine_code & 0x2000000;
	decode_ins.link = decode_ins.machine_code & 0x1000000;
	if (decode_ins.rm_register_used)
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
	decode_ins.rd_number = PC;
	decode_ins.write_rd = true;
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

uint32_t CPU::Pipeline::barrel_shifter(uint32_t value, uint32_t shift_amount, uint8_t shift_type)
{
	switch (shift_type)
	{
		//LSL
	case 0:
		return (value << shift_amount);
	case 1:
		return (value >> shift_amount);
	case 2:
		return (((int32_t)value) >> shift_amount);
	case 3:
		return ((value >> shift_amount) | (value << (32 - shift_amount)));
	default:
		return (value);
	}

}

void CPU::Pipeline::execute() {
	switch (execute_ins.instruction_code)
	{
	case 0:
		execute_ALU();
		break;
	case 1:
		execute_Memory();
		break;
	case 2:
		execute_Control();
		break;
	case 3:
		no_op();
		break;
	default:
		std::cout << "Problem in IC decode";
		break;
	}

}

void CPU::Pipeline::execute_ALU()
{
}

void CPU::Pipeline::execute_Memory()
{
	if (execute_ins.pre_index) {
		execute_ins.rn_value += execute_ins.offset_amount;
	}
}

void CPU::Pipeline::execute_Control()
{
}

void CPU::Pipeline::memory() {
	if (memory_ins.instruction_code == 1) {
		if (memory_ins.opcode == 1)
			memory_ins.result = CPU->read(memory_ins.rn_value);
		else if (memory_ins.opcode == 0)
			CPU->write(memory_ins.rd_value, memory_ins.rn_value);
	}
}

void CPU::Pipeline::writeback() {
	if (writeback_ins.write_rn == true)
		CPU->registers[writeback_ins.rn_number] = writeback_ins.rn_value + writeback_ins.offset_amount;
	if (writeback_ins.write_rd == true)
		CPU->registers[writeback_ins.rd_number] = writeback_ins.result;
}