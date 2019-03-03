#pragma once
#include <vector>
#include <bitset>
#include <iostream>
#include <map>

class Memory {
private:
	class Line {
	private:
		bool empty = true;
		bool dirty = false;
		int age = 0;
		uint32_t tag = 0; // 0 = unset
		std::vector<uint32_t> mem_array; // array of words in memory

	public:
		Line(int words): mem_array(std::vector<uint32_t>(words, 0)) {};
		void set_dirty(bool new_dirty) { dirty = new_dirty; }
		bool is_dirty() { return dirty; }
		bool is_empty() { return empty; }
		int get_age() { return age; }
		void age_incr() { age++; }
		void age_reset() { age = 0; }
		uint32_t get_tag() { return tag; }
		void set_tag(uint32_t l_tag) { tag = l_tag; }
		std::vector<uint32_t> read_block() { return mem_array; }

		int read(uint32_t l_tag, uint32_t offset); // returns word if hit, -1 if not
		bool write(uint32_t word, uint32_t l_tag, uint32_t offset); // returns true if hit, false if miss
		bool write(std::vector<uint32_t> block, uint32_t l_tag, uint32_t offset); // same as above but works with whole line
		std::vector<uint32_t>* read_block(uint32_t l_tag, uint32_t offset);
		std::vector<uint32_t> evict(); // reads out entire line, then erases memory
		void display();
	};

	class Set {
	private:
		std::vector<Line> lines;

	public:
		Set(short ways, int words): lines(std::vector<Line>(ways, Line(words))) {};

		int read(uint32_t tag, uint32_t offset);
		bool write(uint32_t word, uint32_t tag, uint32_t offset);
		bool write(std::vector<uint32_t> block, uint32_t tag, uint32_t offset);
		std::vector<uint32_t>* read_block(uint32_t tag, uint32_t offset);
		Line* find_LRU();
		void display();
	};

	Memory* next_level = 0;
	short latency;
	std::map<uint32_t, short> timers; // each address access has its own timer to track latency
	long size; // memory size in bytes
	int line_length; // line size in bytes
	short ways; // associativity
	int word_size; // word size of the associated processor in bits

	int line_n; // number of lines
	int set_n; // number of sets
	int word_n; // number of words in line
	int index_n; // number of bits required to address sets
	int offset_n; // number of bits in offset field
	int tag_n; // number of bits in tag field

	std::vector<Set> sets;

public:
	Memory(short l_latency, short l_ways, long l_size, int l_line_length, int l_word_size);
	void attach_memory(Memory* l_mem) { next_level = l_mem; }
	void increment_timer(uint32_t addr) { timers[addr]++; }
	void reset_timer(uint32_t addr) { timers[addr] = 0; }
	short get_latency() { return latency; }

	uint32_t read(uint32_t index, uint32_t tag, uint32_t offset);
	void write(uint32_t word, uint32_t index, uint32_t tag, uint32_t offset);
	void write(std::vector<uint32_t> block, uint32_t index, uint32_t tag, uint32_t offset);
	std::vector<uint32_t>* read_block(uint32_t index, uint32_t tag, uint32_t offset);
	uint32_t decode_offset(uint32_t addr);
	uint32_t decode_index(uint32_t addr);
	uint32_t decode_tag(uint32_t addr);
	short query_timer(uint32_t addr);
	void print();
	void display(); // prints the contents of memory to screen
};