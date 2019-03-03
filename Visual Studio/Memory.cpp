#include "pch.h"
#include "Memory.h"

// Memory methods

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

short Memory::query_timer(uint32_t addr) {
	if (!timers.count(addr)) { // address has never been accessed
		timers[addr] = 0;
	}
	return timers[addr];
}

uint32_t Memory::read(uint32_t index, uint32_t tag, uint32_t offset) {
	uint32_t word = sets[index].read(tag, offset);
	if (word != -1) return word;
	else {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t>* lower_data = next_level->read_block(index, tag, offset);
		lru->write(*lower_data, tag, offset);
		lru->set_dirty(false);
		return (*lower_data)[offset];
	}
}

std::vector<uint32_t>* Memory::read_block(uint32_t index, uint32_t tag, uint32_t offset) {
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

void Memory::write(uint32_t word, uint32_t index, uint32_t tag, uint32_t offset) {
	bool hit = sets[index].write(word, tag, offset);
	if (!hit) {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t>* lower_data = next_level->read_block(index, tag, offset);
		lru->write(*lower_data, tag, offset);
		lru->write(word, tag, offset);
		lru->set_dirty(false);
	}
}

void Memory::write(std::vector<uint32_t> block, uint32_t index, uint32_t tag, uint32_t offset) {
	bool hit = sets[index].write(block, tag, offset);
	if (!hit) {
		Memory::Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> temp = lru->evict();
			next_level->write(temp, index, tag, offset);
		}

		std::vector<uint32_t>* lower_data = next_level->read_block(index, tag, offset);
		lru->write(*lower_data, tag, offset);
		lru->write(block, tag, offset);
		lru->set_dirty(false);
	}
}

uint32_t Memory::decode_index(uint32_t addr) {
	/*
	mask = 1s that fill width of index_n
	mask = 2^(index_n) - 1
	index_n = log2 set_n
	mask = 2^(log2 set_n) - 1 = set_n - 1
	*/

	uint32_t mask = (set_n - 1) << offset_n; // generate 1s and shift to proper location
	return (addr & mask) >> offset_n; // extract index and shift right
}

uint32_t Memory::decode_tag(uint32_t addr) {
	/*
	mask = 1s that fill width of tag_n
	mask = 2^(tag_n) - 1
	*/

	uint32_t mask = ((int)pow(2, tag_n) - 1) << (offset_n + index_n);
	return (addr & mask) >> (offset_n + index_n);
}

uint32_t Memory::decode_offset(uint32_t addr) {
	/*
	mask = 1s that fill width of offset_n
	mask = 2^(offset_n) - 1
	offset_n = log2 word_n
	2^(log2 word_n) - 1 = word_n - 1
	*/

	uint32_t mask = word_n - 1;
	return addr & mask;
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

void Memory::display() {
	for (uint32_t i = 0; i < sets.size(); i++) {
		std::cout << "Set index: " << std::hex << i << std::endl;
		sets[i].display();
	}
}

//Set methods

bool Memory::Set::write(uint32_t word, uint32_t tag, uint32_t offset) {
	bool hit = false;

	for (Line line: lines) {
		hit = line.write(word, tag, offset);
		if (hit) {
			for (uint32_t w : line.read_block()) {
				std::cout << w << " ";
			}
			line.set_dirty(true);
			line.age_reset();
			break;
		}
	}

	if (hit) {
		for (Line line: lines) {
			if (!line.is_empty() && tag != line.get_tag()) line.age_incr();
		}
	}

	return hit;
}

bool Memory::Set::write(std::vector<uint32_t> block, uint32_t tag, uint32_t offset) {
	bool hit = false;

	for (Line line: lines) {
		hit = line.write(block, tag, offset);
		if (hit) {
			line.set_dirty(true);
			line.age_reset();
			break;
		}
	}

	if (hit) {
		for (Line line: lines) {
			if (!line.is_empty() && tag != line.get_tag()) line.age_incr();
		}
	}

	return hit;
}

int Memory::Set::read(uint32_t tag, uint32_t offset) {
	int word = -1;

	for (Line line: lines) {
		word = line.read(tag, offset);
		if (word != -1) { // if hit
			line.age_reset();
			break;
		}
	}

	if (word != -1) { // if hit: update age of other lines
		for (Line line: lines) {
			if (!line.is_empty() && tag != line.get_tag()) line.age_incr();
		}
	}

	return word;
}

std::vector<uint32_t>* Memory::Set::read_block(uint32_t tag, uint32_t offset) {
	std::vector<uint32_t>* block = 0;

	for (Line line: lines) {
		block = line.read_block(tag, offset);
		if (block != 0) { // if hit
			line.age_reset();
			break;
		}
	}

	if (block != 0) { // if hit: update age of other lines
		for (Line line: lines) {
			if (!line.is_empty() && tag != line.get_tag()) line.age_incr();
		}
	}

	return block;
}

Memory::Line* Memory::Set::find_LRU() { // returns pointer to first empty block in set, or if there is no empty block, the oldest block
	Line* cur = 0;
	Line* oldest = &(lines[0]);

	for (uint32_t i = 0; i < lines.size(); i++) {
		cur = &(lines[i]);
		if (cur->is_empty()) return cur;
		if (cur->get_age() > oldest->get_age()) oldest = cur;
	}

	return oldest;
}

void Memory::Set::display() {
	for (Line line: lines) {
		line.display();
	}
}

// Line methods

bool Memory::Line::write(std::vector<uint32_t> block, uint32_t l_tag, uint32_t offset) {
	if (empty) tag = l_tag;
	if (l_tag == tag) {
		mem_array = block;
		empty = false;
		return true;
	}
	return false;
}

std::vector<uint32_t>* Memory::Line::read_block(uint32_t l_tag, uint32_t offset) {
	if (tag == l_tag) return &(mem_array);
	return 0;
}

std::vector<uint32_t> Memory::Line::evict() {
	std::vector<uint32_t> temp = mem_array;
	tag = 0;
	mem_array = std::vector<uint32_t>(mem_array.size(), 0);
	return temp;
}

int Memory::Line::read(uint32_t l_tag, uint32_t offset) {
	if (tag == l_tag) return mem_array[offset];
	return -1;
}

bool Memory::Line::write(uint32_t word, uint32_t l_tag, uint32_t offset) {
	if (empty) tag = l_tag;
	if (l_tag == tag) {
		mem_array[offset] = word;
		empty = false;
		for (uint32_t w : mem_array) std::cout << w << " ";
		return true;
	}
	return false;
}

void Memory::Line::display() {
	std::cout << tag << " ";
	for (uint32_t i = 0; i < mem_array.size(); i++) {
		std::cout << std::hex << mem_array[i] << " ";
	}
	std::cout << std::endl;
}