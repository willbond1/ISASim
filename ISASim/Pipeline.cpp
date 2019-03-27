#include "pch.h"
#include "CPU.h"
#include <set>


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
		fetch_ins.machine_code = cpu->read(next_addr);
		decode();
		execute();
		memory();
		writeback();

		writeback_ins = memory_ins;
		memory_ins = execute_ins;
		if (decode_ins.decoded) {
			execute_ins = decode_ins;
			decode_ins = fetch_ins;
		}
		else
			execute_ins = no_op_ins;

	}
	else { // move pipeline along but don't accept new instructions until empty
		fetch_ins.machine_code = cpu->read(next_addr);
		cpu->clock_incr();
		decode_ins = fetch_ins;
		decode();
		cpu->clock_incr();
		execute_ins = decode_ins;
		execute();
		cpu->clock_incr();
		memory_ins = execute_ins;
		memory();
		cpu->clock_incr();
		writeback_ins = memory_ins;
		writeback();
	}

	cpu->clock_incr();
}


void CPU::Pipeline::flushPipeline()
{
	fetch_ins.machine_code = 0xffffffff;
	decode_ins.machine_code = 0xffffffff;
	fetch_ins.instruction_code = 3;
	decode_ins.instruction_code = 3;
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

	std::set<uint8_t> used_registers;
	if (writeback_ins.rd_register_used)
		used_registers.insert(writeback_ins.rd_number);
	if (writeback_ins.rn_register_used)
		used_registers.insert(writeback_ins.rn_number);
	if (writeback_ins.rm_register_used)
		used_registers.insert(writeback_ins.rm_number);
	if (writeback_ins.rs_register_used)
		used_registers.insert(writeback_ins.rs_number);
	if (memory_ins.rd_register_used)
		used_registers.insert(memory_ins.rd_number);
	if (memory_ins.rn_register_used)
		used_registers.insert(memory_ins.rn_number);
	if (memory_ins.rm_register_used)
		used_registers.insert(memory_ins.rm_number);
	if (memory_ins.rs_register_used)
		used_registers.insert(memory_ins.rs_number);
	if (execute_ins.rd_register_used)
		used_registers.insert(execute_ins.rd_number);
	if (execute_ins.rn_register_used)
		used_registers.insert(execute_ins.rn_number);
	if (execute_ins.rm_register_used)
		used_registers.insert(execute_ins.rm_number);
	if (execute_ins.rs_register_used)
		used_registers.insert(execute_ins.rs_number);
		
		
	if (decode_ins.rd_register_used && (std::find(used_registers.begin(), used_registers.end(), decode_ins.rd_number) != used_registers.end()))
		decode_ins.decoded = false;
	else if (decode_ins.rn_register_used && (std::find(used_registers.begin(), used_registers.end(), decode_ins.rn_number) != used_registers.end()))
		decode_ins.decoded = false;
	else if (decode_ins.rm_register_used && (std::find(used_registers.begin(), used_registers.end(), decode_ins.rm_number) != used_registers.end()))
		decode_ins.decoded = false;
	else if (decode_ins.rs_register_used && (std::find(used_registers.begin(), used_registers.end(), decode_ins.rs_number) != used_registers.end()))
		decode_ins.decoded = false;
	else {
		decode_ins.decoded = true;
		if (decode_ins.rd_register_used) {
			decode_ins.rd_value = cpu->registers[decode_ins.rd_number];
		}
		if (decode_ins.rn_register_used) {
			decode_ins.rn_value = cpu->registers[decode_ins.rn_number];
		}
		if (decode_ins.rm_register_used) {
			decode_ins.rm_value = cpu->registers[decode_ins.rm_number];
			decode_ins.op2_value = decode_ins.rm_value;
		}
		if (decode_ins.rs_register_used) {
			decode_ins.rs_value = cpu->registers[decode_ins.rs_number];
		}
		

	}
}
/*		
		
		uint32_t machine_code;
		uint32_t result; // instruction result (if there is one)
		int32_t offset_amount;
		uint32_t rd_value;
		uint32_t rn_value;
		uint32_t rm_value;
		uint32_t rs_value;
		uint32_t op2_value;
		uint8_t condition_code;
		uint8_t instruction_code;
		uint8_t opcode;
		uint8_t rd_number;
		uint8_t rn_number;
		uint8_t rm_number;
		uint8_t rs_number;
		uint8_t off_shift_type;
		bool write_rn = false;A
		bool write_rd = false;A
		bool pre_index = true;A
		bool add_sub_offset = true;A
		bool rd_register_used = false;A
		bool rn_register_used = false;
		bool rm_register_used = false;
		bool rs_register_used = false;
		bool update_status = false;
		bool link = false;
		bool decoded = false;
*/

void CPU::Pipeline::decode_ALU()
{
	decode_ins.write_rn = false;
	//Always pre-index
	decode_ins.pre_index = true;
	//Always adding signed offset
	decode_ins.add_sub_offset = true;
	//Opcode
	decode_ins.opcode = (decode_ins.machine_code & 0x1e00000) >> 21;
	//Whether writes to register
	//Rd
	decode_ins.write_rd = false;
	decode_ins.rd_register_used = false;
	if (!(decode_ins.opcode >= 8 && decode_ins.opcode <= 11)) {
		decode_ins.write_rd = true;
		decode_ins.rd_register_used = true;
		decode_ins.rd_number = (decode_ins.machine_code & 0xF0000) >> 16;
	}
	
	//Rn (OP1)
	decode_ins.rn_register_used = true;
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

		decode_ins.off_shift_type = (decode_ins.machine_code & 0x60) >> 5;
	}
	else {
		decode_ins.op2_value = decode_ins.machine_code & 0xff;
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
		uint32_t sign_extend = decode_ins.machine_code & 0x800 ? 0xfffff000 : 0;
		decode_ins.offset_amount = decode_ins.machine_code & 0xfff | sign_extend;
	}
	//Always Immediate Shift Amount
	decode_ins.rs_register_used = false;
}

void CPU::Pipeline::decode_Control()
{
	decode_ins.rn_register_used = false;
	decode_ins.rs_register_used = false;
	decode_ins.rd_register_used = false;
	decode_ins.rm_register_used = decode_ins.machine_code & 0x2000000;
	decode_ins.link = decode_ins.machine_code & 0x1000000;
	if (decode_ins.rm_register_used)
		decode_ins.rm_number = decode_ins.machine_code & 0xF;
	else {
		uint32_t sign_extend = decode_ins.machine_code & 0x800000 ? 0xff000000 : 0;
		decode_ins.offset_amount = (decode_ins.machine_code & 0xffffff) | sign_extend;
	}
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
		return false;
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
	if(condition_valid(execute_ins.condition_code))
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
	uint64_t unsigned_sum;
	int64_t signed_sum;
	switch (execute_ins.opcode)
	{
	case 2:
	case 4: //ADD
	case 11: //CMN
	case 10:
		unsigned_sum = (uint64_t)execute_ins.rn_value + (uint64_t)execute_ins.op2_value;
		signed_sum = (int64_t)(int32_t)execute_ins.rn_value + (int64_t)(int32_t)execute_ins.op2_value;
		execute_ins.result = unsigned_sum & 0xffffffff;
		if (execute_ins.update_status) {
			cpu->C_flag = (((uint32_t)execute_ins.result) == unsigned_sum) ? 0 : 1;
			cpu->V_flag = (((int32_t)execute_ins.result) == signed_sum) ? 0 : 1;
			cpu->Z_flag = execute_ins.result == 0;
			cpu->N_flag = execute_ins.result & 0x80000000;
		}
		break;
	default:
		break;
	}
}

void CPU::Pipeline::execute_Memory()
{
	if (execute_ins.pre_index) {
		execute_ins.rn_value += execute_ins.offset_amount;
	}
}

void CPU::Pipeline::execute_Control()
{
	flushPipeline();
	cpu->registers[PC] -= 2;
	cpu->registers[PC] += execute_ins.offset_amount;
}

void CPU::Pipeline::memory() {
	if (memory_ins.instruction_code == 1) {
		if (memory_ins.opcode == 1)
			memory_ins.result = cpu->read(memory_ins.rn_value);
		else if (memory_ins.opcode == 0)
			cpu->write(memory_ins.rd_value, memory_ins.rn_value);
	}
}

void CPU::Pipeline::writeback() {
	if (writeback_ins.write_rn == true)
		cpu->registers[writeback_ins.rn_number] = writeback_ins.rn_value + writeback_ins.offset_amount;
	if (writeback_ins.write_rd == true)
		cpu->registers[writeback_ins.rd_number] = writeback_ins.result;
}

void CPU::Pipeline::display_contents() {
	std::cout << "Pipeline contents:" << std::endl
		<< "Fetch step: " << fetch_ins.machine_code << std::endl
		<< "Decode step: " << decode_ins.machine_code << std::endl
		<< "Execute step: " << execute_ins.machine_code << std::endl
		<< "Memory step: " << memory_ins.machine_code << std::endl
		<< "Writeback step: " << writeback_ins.machine_code << std::endl;
}