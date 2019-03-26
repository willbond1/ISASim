#include "pch.h"
#include "CPU.h"


CPU::Pipeline::Pipeline(CPU *l_cpu) : cpu(l_cpu)
{
}


CPU::Pipeline::~Pipeline()
{
}

void CPU::Pipeline::step(bool pipe, bool cache, uint32_t next_inst) // next_inst passed in externally, NOT read from memory
{
	Memory *mem_pointer = cpu->mem; // pointer to memory to read/write from
	uint32_t next_addr = cpu->registers[PC]; // next instruction address
	uint32_t result = writeback_ins.result;

	if (!cache) { // directly access memory
		while (mem_pointer->get_next_level() != 0) { // get pointer to main memory
			mem_pointer = mem_pointer->get_next_level();
		}
	}

	while (mem_pointer->query_timer(next_addr) < mem_pointer->get_latency()) {
		; // should write to memory, if memory_ins
	}

	if (pipe) { // normal pipeline

	} else { // move pipeline along but don't accept new instructions until empty

	}

	cpu->clock_incr();
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
		bool offset_register;
		bool offset_shift_register;
*/

void CPU::Pipeline::decode_ALU()
{
	//Always post-index
	decode_ins.pre_index = false;
	//Always adding signed offset
	decode_ins.add_sub_offset = true;
	//Opcode
	decode_ins.opcode = (decode_ins.machine_code & 0x1e00000) >> 21;
	//Whether writes to register
	decode_ins.write_back = !(decode_ins.opcode >= 8 && decode_ins.opcode <= 11);
	//Rd
	decode_ins.rd_number = (decode_ins.machine_code & 0xF0000) >> 16;
	//Rn (OP1)
	decode_ins.rn_number = (decode_ins.machine_code & 0xF000) >> 12;
	//S
	decode_ins.update_status = decode_ins.machine_code & 0x100000;
	//I
	decode_ins.offset_register = decode_ins.machine_code & 0x2000000;

	if (decode_ins.offset_register) {
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
		decode_ins.offset_shift_register = decode_ins.machine_code & 0x10;

		if (decode_ins.offset_shift_register)
			decode_ins.rs_number = (decode_ins.machine_code & 0xf00) >> 8;
		else
			decode_ins.offset_shift_register = false;

		decode_ins.off_shift_type = (decode_ins.machine_code & 0x60) >> 5;
	}
	else {
		decode_ins.offset_shift_register = false;
	}
}

void CPU::Pipeline::decode_Memory()
{
	//P
	decode_ins.pre_index = decode_ins.machine_code & 0x1000000;
	//U
	decode_ins.add_sub_offset = decode_ins.machine_code & 0x800000;
	//W
	decode_ins.write_back = decode_ins.machine_code & 0x400000;
	//L
	decode_ins.opcode = decode_ins.machine_code & 0x200000;
	//Rd
	decode_ins.rd_number = (decode_ins.machine_code & 0x1E000) >> 13;
	//Rn (OP1)
	decode_ins.rn_number = (decode_ins.machine_code & 0x1E0000) >> 17;
	//I
	decode_ins.offset_register = decode_ins.machine_code & 0x2000000;
	if (decode_ins.offset_register) {
		//Rm
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
		//Shift type
		decode_ins.off_shift_type = (decode_ins.machine_code & 0xc0) >> 5;
	}
	//Always Immediate Shift Amount
	decode_ins.offset_shift_register = false;
}

void CPU::Pipeline::decode_Control()
{
	decode_ins.offset_register = decode_ins.machine_code & 0x2000000;
	decode_ins.link = decode_ins.machine_code & 0x1000000;
	if (decode_ins.offset_register)
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
	decode_ins.rd_number = PC;
	decode_ins.write_back = true;
}

void CPU::Pipeline::no_op()
{
}

bool CPU::Pipeline::condition_valid(uint8_t code)
{
	switch (code)
	{
	case 0:
		return cpu->Z_flag;
	case 1:
		return !cpu->Z_flag;
	case 2:
		return cpu->C_flag;
	case 3:
		return !cpu->C_flag;
	case 4:
		return cpu->N_flag;
	case 5:
		return !cpu->N_flag;
	case 6:
		return cpu->V_flag;
	case 7:
		return !cpu->V_flag;
	case 8:
		return cpu->C_flag && !cpu->Z_flag;
	case 9:
		return !cpu->C_flag || cpu->Z_flag;
	case 10:
		return cpu->N_flag == cpu->V_flag;
	case 11:
		return cpu->N_flag != cpu->V_flag;
	case 12:
		return !cpu->Z_flag && (cpu->N_flag == cpu->V_flag);
	case 13:
		return cpu->Z_flag || (cpu->N_flag != cpu->V_flag);
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

void CPU::Pipeline::display_contents() {
	std::cout << "Pipeline contents:" << std::endl
		<< "Fetch step: " << fetch_ins.machine_code << std::endl
		<< "Decode step: " << decode_ins.machine_code << std::endl
		<< "Execute step: " << execute_ins.machine_code << std::endl
		<< "Memory step: " << memory_ins.machine_code << std::endl
		<< "Writeback step: " << writeback_ins.machine_code << std::endl;
}