#pragma once
#include <vector>
#include <bitset>
#include <iostream>
#include <map>

class Memory {
private:
	friend class Interface;

	class Line {
	private:
		bool empty;
		bool dirty = false;
		uint32_t age = 0;
		uint32_t tag = 0;
		std::vector<uint32_t> mem_array; // array of words in memory

	public:
		Line(int words, bool is_RAM): mem_array(std::vector<uint32_t>(words, 0)), empty(!is_RAM) {};
		void set_dirty(bool new_dirty) { dirty = new_dirty; }
		bool is_dirty() { return dirty; }
		bool is_empty() { return empty; }
		void set_empty(bool l_empty) { empty = l_empty; }
		uint32_t get_age() { return age; }
		void age_incr() { age++; }
		void age_decr() { age--; }
		void age_reset() { age = 0; }
		uint32_t get_tag() { return tag; }
		void set_tag(uint32_t l_tag) { tag = l_tag; }
		uint32_t read(uint32_t offset) { return mem_array[offset]; }
		std::vector<uint32_t> read() { return std::vector<uint32_t>(mem_array); }
		void write(uint32_t word, uint32_t offset) { mem_array.at(offset) = word; empty = false; }
		void write(std::vector<uint32_t>& words) { mem_array.assign(words.begin(), words.end()); empty = false; }
		bool is_hit(uint32_t l_tag) { return ((l_tag == tag) && !is_empty()); }

		std::vector<uint32_t> evict(); // reads out entire line, then erases memory
		void display();
	};

	class Set {
	private:
		std::vector<Line> lines;

	public:
		Set(uint32_t ways, uint32_t words, bool is_RAM): lines(std::vector<Line>(ways, Line(words, is_RAM))) {
			if(is_RAM)
				for (std::vector<Line>::size_type i = 0; i < lines.size(); i++) {
					lines[i].set_tag(i);
					std::cout << "LINE: " << i << " " << lines[i].get_tag() << std::endl;
				}
		};

		uint32_t read(uint32_t tag, uint32_t offset);
		std::vector<uint32_t> read(uint32_t tag);
		void write(uint32_t word, uint32_t tag, uint32_t offset);
		void write(std::vector<uint32_t>& block, uint32_t tag);
		//void write(std::vector<uint32_t> block, uint32_t tag);
		Line* is_hit(uint32_t tag);
		void lru_incr(Line * recent_line);
		Line* find_LRU();
		void display();
	};

	Memory* next_level = 0;
	uint32_t latency;
	std::map<uint32_t, uint32_t> timers; // each address access has its own timer to track latency
	uint32_t size; // memory size in bytes
	uint32_t line_length; // line size in bytes
	uint32_t ways; // associativity
	uint32_t word_size; // word size of the associated processor in bits

	uint32_t line_n; // number of lines
	uint32_t set_n; // number of sets
	uint32_t word_n; // number of words in line
	uint32_t index_n; // number of bits required to address sets
	uint32_t offset_n; // number of bits in offset field
	uint32_t tag_n; // number of bits in tag field

	bool is_RAM;

	std::vector<Set> sets;

public:
	Memory(uint32_t l_latency, uint32_t l_ways, uint32_t l_size, uint32_t l_line_length, uint32_t l_word_size, bool is_RAM);
	void attach_memory(Memory* l_mem) { next_level = l_mem; }
	void increment_timer(uint32_t addr) { timers[addr]++; }
	void reset_timer(uint32_t addr) { timers[addr] = 0; }
	uint32_t get_latency() { return latency; }

	uint32_t read(uint32_t addr);
	std::vector<uint32_t> read_block(uint32_t addr);
	void write(uint32_t word, uint32_t addr);
	void write(std::vector<uint32_t>& block, uint32_t addr);
	//void write(std::vector<uint32_t> block, uint32_t addr);
	uint32_t encode_addr(uint32_t tag, uint32_t index, uint32_t offset);
	Line* is_hit(uint32_t index, uint32_t tag);
	uint32_t decode_offset(uint32_t addr);
	uint32_t decode_index(uint32_t addr);
	uint32_t decode_tag(uint32_t addr);
	uint32_t query_timer(uint32_t addr);
	void print();
	void display(); // prints the contents of memory to screen
};