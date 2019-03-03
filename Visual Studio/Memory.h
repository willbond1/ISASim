#pragma once
#include <vector>
#include <bitset>
#include <iostream>
#include <map>

class Memory {
private:
	class Line {
	private:
		bool dirty = false;
		int age = 0;
		uint32_t tag = 0; // 0 = unset
		std::vector<uint32_t> mem_array; // array of words in memory

	public:
		Line(int words): mem_array(std::vector<uint32_t>(words, 0)) {};
		void set_dirty(bool new_dirty) { dirty = new_dirty; }
		bool is_dirty() { return dirty; }
		int get_age() { return age; }
		void age_incr() { age++; }
		void age_reset() { age = 0; }
		uint32_t get_tag() { return tag; }
		void set_tag(uint32_t l_tag) { tag = l_tag; }
		std::vector<uint32_t> retrieve_block() { return mem_array; }

		int read(int l_tag, int offset); // returns word if hit, -1 if not
		bool write(uint32_t word, int l_tag, int offset); // returns true if hit, false if miss
		bool write(std::vector<uint32_t> block, int l_tag, int offset); // same as above but works with whole line
		std::vector<uint32_t>* read_block(int l_tag, int offset);
		std::vector<uint32_t> evict(); // reads out entire line, then erases memory
	};

	class Set {
	private:
		std::vector<Line> lines;

	public:
		Set(short ways, int words): lines(std::vector<Line>(ways, Line(words))) {};

		int read(int tag, int offset);
		bool write(uint32_t word, int tag, int offset);
		bool write(std::vector<uint32_t> block, int tag, int offset);
		std::vector<uint32_t>* read_block(int tag, int offset);
		Line* find_LRU();
	};

	Memory* next_level = 0;
	short latency;
	std::map<long, short> timers; // each address access has its own timer to track latency
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
	void increment_timer(long addr) { timers[addr]++; }
	void reset_timer(long addr) { timers[addr] = 0; }
	short get_latency() { return latency; }

	int read(int index, int tag, int offset);
	void write(uint32_t word, int index, int tag, int offset);
	void write(std::vector<uint32_t> block, int index, int tag, int offset);
	std::vector<uint32_t>* read_block(int index, int tag, int offset);
	int decode_offset(long addr);
	int decode_index(long addr);
	int decode_tag(long addr);
	short query_timer(long addr);
	void print();
};