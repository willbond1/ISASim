#include "pch.h"

// Memory methods

Memory::Memory(uint32_t l_latency, uint32_t l_ways, uint32_t l_size, uint32_t l_line_length, uint32_t l_word_size, bool is_RAM):
	latency(l_latency), ways(l_ways), size(l_size), line_length(l_line_length), word_size(l_word_size) {

	line_n = size / line_length;
	set_n = ceil(line_n / ways);
	index_n = ceil(log2((float)set_n));
	word_n = (line_length * 8) / word_size;
	offset_n = ceil(log2((float)word_n));
	tag_n = word_size - (offset_n + index_n);

	for (size_t i = 0; i < set_n; i++) {
		sets.push_back(Set(ways, word_n, is_RAM));
	}
}

uint32_t Memory::query_timer(uint32_t addr) {
	if (!timers.count(addr)) { // address has never been accessed
		timers[addr] = 0;
	}
	return timers[addr];
}

bool Memory::is_hit(uint32_t index, uint32_t tag) {
	return sets[index].is_hit(tag);
}

uint32_t Memory::read(uint32_t addr) {
	uint32_t index = decode_index(addr);
	uint32_t tag = decode_tag(addr);
	uint32_t offset = decode_offset(addr);

	if (is_hit(index, tag) || next_level == 0) {
		return sets[index].read(tag, offset);
	} else {
		Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> old = lru->evict();
			next_level->write(old, addr);
		}

		std::vector<uint32_t> new_block = next_level->read_block(addr);
		lru->write(new_block);
		lru->set_dirty(false);
		return new_block[offset];
	}
}

std::vector<uint32_t> Memory::read_block(uint32_t addr) {
	uint32_t index = decode_index(addr);
	uint32_t tag = decode_tag(addr);
	uint32_t offset = decode_offset(addr);

	if (is_hit(index, tag) || next_level == 0) {
		return sets[index].read(tag);
	} else {
		Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> old = lru->evict();
			next_level->write(old, addr);
		}

		std::vector<uint32_t> new_block = next_level->read_block(addr);
		lru->write(new_block);
		lru->set_dirty(false);
		return new_block;
	}
}

void Memory::write(uint32_t word, uint32_t addr) {
	uint32_t index = decode_index(addr);
	uint32_t tag = decode_tag(addr);
	uint32_t offset = decode_offset(addr);

	if (is_hit(index, tag) || next_level == 0) {
		sets[index].write(word, tag, offset);
	} else {
		Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> old = lru->evict();
			next_level->write(old, addr);
		}

		std::vector<uint32_t> new_block = next_level->read_block(addr);
		lru->write(new_block);
		lru->write(word, offset);
		lru->set_dirty(true);
	}
}

void Memory::write(std::vector<uint32_t> block, uint32_t addr) {
	uint32_t index = decode_index(addr);
	uint32_t tag = decode_tag(addr);
	uint32_t offset = decode_offset(addr);

	if (is_hit(index, tag) || next_level == 0) {
		sets[index].write(block, tag);
	} else {
		Line* lru = sets[index].find_LRU();
		if (lru->is_dirty()) {
			std::vector<uint32_t> old = lru->evict();
			next_level->write(old, addr);
		}

		lru->write(block);
		lru->set_dirty(true);
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

bool Memory::Set::is_hit(uint32_t tag) {
	for (Line line: lines) {
		if (line.is_hit(tag) && !line.is_empty())
			return true;
	}
	return false;
}

void Memory::Set::write(uint32_t word, uint32_t tag, uint32_t offset) {
	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			line.write(word, offset);
			line.age_reset();
		} else if (!line.is_empty())
			line.age_incr();
	}
}

void Memory::Set::write(std::vector<uint32_t> block, uint32_t tag) {
	for (Line line: lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			line.write(block);
			line.age_reset();
		} else if (!line.is_empty())
			line.age_incr();
	}
}

uint32_t Memory::Set::read(uint32_t tag, uint32_t offset) {
	uint32_t word = 0;

	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			word = line.read(offset);
			line.age_reset();
		} else if (!line.is_empty())
			line.age_incr();
	}

	return word;
}

std::vector<uint32_t> Memory::Set::read(uint32_t tag) {
	std::vector<uint32_t> block = std::vector<uint32_t>();

	for (Line line: lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			block = line.read();
			line.age_reset();
		} else if (!line.is_empty())
			line.age_incr();
	}

	return block;
}

Memory::Line* Memory::Set::find_LRU() { // returns pointer to first empty block in set, or if there is no empty block, the oldest block
	Line* cur = 0;
	Line* oldest = &(lines[0]);

	for (uint32_t i = 0; i < lines.size(); i++) {
		cur = &(lines[i]);
		if (cur->is_empty())
			return cur;
		if (cur->get_age() > oldest->get_age())
			oldest = cur;
	}

	return oldest;
}

void Memory::Set::display() {
	for (Line line: lines) {
		line.display();
	}
}

// Line methods

std::vector<uint32_t> Memory::Line::evict() {
	std::vector<uint32_t> temp = mem_array;
	empty = true;
	mem_array = std::vector<uint32_t>(mem_array.size(), 0);
	return temp;
}

void Memory::Line::display() {
	std::cout << tag << " ";
	for (uint32_t i = 0; i < mem_array.size(); i++) {
		std::cout << std::hex << mem_array[i] << " ";
	}
	std::cout << std::endl;
}