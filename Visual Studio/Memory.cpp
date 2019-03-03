#include "pch.h"
#include "Memory.h"

Memory::Memory(short l_latency, short l_ways, long l_size, int l_line_length, int l_word_size):
	latency(l_latency), ways(l_ways), size(l_size), line_length(l_line_length), word_size(l_word_size) {

	line_n = size / line_length;
	set_n = ceil(line_n / ways);
	index_n = ceil(log2((float)set_n));
	word_n = (line_length * 8) / word_size;
	offset_n = ceil(log2((float)word_n));
	tag_n = word_size - (offset_n + index_n);

	for (int i = 0; i < set_n; i++) {
		sets.push_back(Set(ways, word_n));
	}
}

int Memory::read(int index, int tag, int offset) {
	int word = sets[index].read(tag, offset);
	if (word != -1) return word;
	else {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t> lower_data = *(next_level->read_block(index, tag, offset));
		lru->write(lower_data, tag, offset);
		lru->set_dirty(false);
		return lower_data[offset];
	}
}

void Memory::write(uint32_t word, int index, int tag, int offset) {
	int hit = sets[index].write(word, tag, offset);
	if (!hit) {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t> lower_data = *(next_level->read_block(index, tag, offset));
		lru->write(lower_data, tag, offset);
		lru->write(word, tag, offset);
		lru->set_dirty(false);
	}
}

void Memory::write(std::vector<uint32_t> block, int index, int tag, int offset) {
	int hit = sets[index].write(block, tag, offset);
	if (!hit) {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t> lower_data = *(next_level->read_block(index, tag, offset));
		lru->write(lower_data, tag, offset);
		lru->write(block, tag, offset);
		lru->set_dirty(false);
	}
}

std::vector<uint32_t>* Memory::read_block(int index, int tag, int offset) {
	std::vector<uint32_t>* block = sets[index].read_block(tag, offset);
	if (block != 0) return block;
	else {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t>* lower_data = next_level->read_block(index, tag, offset);
		lru->write(*lower_data, tag, offset);
		lru->set_dirty(false);
		return lower_data;
	}
}

bool Memory::Set::write(std::vector<uint32_t> block, int tag, int offset) {
	bool hit = false;
	Line *cur = 0;
	short ways = lines.size();

	for (int i = 0; i < ways; i++) {
		cur = &(lines[i]);
		hit = cur->write(block, tag, offset);
		if (hit) {
			cur->set_dirty(true);
			cur->age_reset();
			break;
		}
	}

	if (hit) {
		for (int i = 0; i < ways; i++) {
			cur = &(lines[i]);
			if (tag != cur->get_tag()) {
				cur->age_incr();
			}
		}
	}

	return hit;
}

std::vector<uint32_t>* Memory::Set::read_block(int tag, int offset) {
	std::vector<uint32_t>* block = 0;
	Line *cur = 0;
	short ways = lines.size();

	for (int i = 0; i < ways; i++) {
		cur = &(lines[i]);
		block = cur->read_block(tag, offset);
		if (block != 0) { // if hit
			cur->age_reset();
			break;
		}
	}

	if (block != 0) { // if hit: update age of other lines
		for (int i = 0; i < ways; i++) {
			cur = &(lines[i]);
			if (tag != cur->get_tag()) {
				cur->age_incr();
			}
		}
	}

	return block;
}

bool Memory::Line::write(std::vector<uint32_t> block, int l_tag, int offset) {
	if (tag == 0) tag = l_tag;
	if (l_tag == tag) {
		mem_array = block;
		return true;
	}
	return false;
}

std::vector<uint32_t>* Memory::Line::read_block(int l_tag, int offset) {
	if (tag == l_tag) return &(mem_array);
	return 0;
}

void Memory::print() { // print cache stats for debugging
	std::cout << "Latency: " << latency << std::endl
		<< "Size in bytes: " << size << std::endl
		<< "Line length in bytes: " << line_length << std::endl
		<< "Associativity: " << ways << std::endl
		<< "Number of lines: " << line_n << std::endl
		<< "Number of sets: " << set_n << std::endl
		<< "Number of words in a line: " << word_n << std::endl
		<< "Number of bits in index field: " << index_n << std::endl
		<< "Number of bits in offset field: " << offset_n << std::endl
		<< "Number of bits in tag field: " << tag_n << std::endl;
}

int Memory::decode_index(long addr) {
	/*
	mask = 1s that fill width of index_n
	mask = 2^(index_n) - 1
	index_n = log2 set_n
	mask = 2^(log2 set_n) - 1 = set_n - 1
	*/

	int mask = (set_n - 1) << offset_n; // generate 1s and shift to proper location
	return (addr & mask) >> offset_n; // extract index and shift right
}

int Memory::decode_tag(long addr) {
	/*
	mask = 1s that fill width of tag_n
	mask = 2^(tag_n) - 1
	*/

	int mask = ((int)pow(2, tag_n) - 1) << (offset_n + index_n);
	return (addr & mask) >> (offset_n + index_n);
}

int Memory::decode_offset(long addr) {
	/*
	mask = 1s that fill width of offset_n
	mask = 2^(offset_n) - 1
	offset_n = log2 word_n
	2^(log2 word_n) - 1 = word_n - 1
	*/

	int mask = word_n - 1;
	return addr & mask;
}

int Memory::Set::read(int tag, int offset) {
	int word = -1;
	Line *cur = 0;
	short ways = lines.size();

	for (int i = 0; i < ways; i++) {
		cur = &(lines[i]);
		word = cur->read(tag, offset);
		if (word != -1) { // if hit
			cur->age_reset();
			break;
		}
	}

	if (word != -1) { // if hit: update age of other lines
		for (int i = 0; i < ways; i++) {
			cur = &(lines[i]);
			if (tag != cur->get_tag()) {
				cur->age_incr();
			}
		}
	}

	return word;
}

bool Memory::Set::write(uint32_t word, int tag, int offset) {
	bool hit = false;
	Line *cur = 0;
	short ways = lines.size();

	for (int i = 0; i < ways; i++) {
		cur = &(lines[i]);
		hit = cur->write(word, tag, offset);
		if (hit) {
			cur->set_dirty(true);
			cur->age_reset();
			break;
		}
	}

	if (hit) {
		for (int i = 0; i < ways; i++) {
			cur = &(lines[i]);
			if (tag != cur->get_tag()) {
				cur->age_incr();
			}
		}
	}

	return hit;
}

Memory::Line* Memory::Set::find_LRU() {
	Line *cur = 0;
	Line *oldest = &(lines[0]);

	for (int i = 0; i < lines.size(); i++) {
		cur = &(lines[i]);
		if (cur->get_tag() == 0) return cur;
		if (cur->get_age() > oldest->get_age()) oldest = cur;
	}

	return oldest;
}

std::vector<uint32_t> Memory::Line::evict() {
	std::vector<uint32_t> temp = mem_array;
	tag = 0;
	mem_array = std::vector<uint32_t>(mem_array.size(), 0);
	return temp;
}

int Memory::Line::read(int l_tag, int offset) {
	if (tag == l_tag) return mem_array[offset];
	return -1;
}

bool Memory::Line::write(uint32_t word, int l_tag, int offset) {
	if (tag == 0) tag = l_tag;
	if (l_tag == tag) {
		mem_array[offset] = word;
		return true;
	}
	return false;
}