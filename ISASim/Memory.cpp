#include "pch.h"

// Memory methods

Memory::Memory(uint32_t l_latency, uint32_t l_ways, uint32_t l_size, uint32_t l_line_length, uint32_t l_word_size, bool is_RAM) :
	latency(l_latency), ways(l_ways), size(l_size), line_length(l_line_length), word_size(l_word_size) {

	line_n = size / line_length;
	set_n = ceil((float)line_n / (float)ways);
	index_n = ceil(log2((float)set_n));
	word_n = (line_length * 8) / word_size;
	offset_n = ceil(log2((float)word_n));
	tag_n = word_size - (offset_n + index_n);

	for (size_t i = 0; i < set_n; i++) {
		sets.push_back(Set(ways, word_n, is_RAM));
	}
}

uint32_t Memory::query_timer(uint32_t addr) {
	if (timers.count(addr) == 0) { // address has never been accessed
		timers[addr] = 0;
	}
	return timers[addr];
}

Memory::Line* Memory::is_hit(uint32_t index, uint32_t tag) {
	return sets[index].is_hit(tag);
}

uint32_t Memory::read(uint32_t addr) {
	clock++;
	if (query_timer(addr) == latency) {
		uint32_t index = decode_index(addr);
		uint32_t tag = decode_tag(addr);
		uint32_t offset = decode_offset(addr);
		Line *hitline = is_hit(index, tag);
		if (hitline) {
			sets[index].lru_incr(hitline);
			return hitline->read(offset);
		}
		else {
			Line* lru = sets[index].find_LRU();
			if (lru->is_dirty()) {
				std::vector<uint32_t> old = lru->evict();
				uint32_t old_addr = encode_addr(lru->get_tag(), index, 0);
				Line * hit_line = next_level->is_hit(next_level->decode_index(old_addr), next_level->decode_tag(old_addr));
				if (hit_line)
					hit_line->write(old);
				else {
					while(next_level->query_timer(old_addr) < next_level->get_latency())
						next_level->write(old, old_addr);
					next_level->write(old, old_addr);
					next_level->reset_timer(old_addr);
				}
			}

			std::vector<uint32_t> new_block;
			while(next_level->query_timer(addr) < next_level->get_latency())
				new_block = next_level->read_block(addr);
			new_block = next_level->read_block(addr);
			next_level->reset_timer(addr);

			lru->write(new_block);
			lru->set_tag(tag);
			lru->set_dirty(false);
			lru->set_empty(false);
			sets[index].lru_incr(lru);
			std::cout << "Read complete." << std::endl;
			return new_block[offset];
		}
	} else {
		std::cout << "Waiting..." << std::endl;
		increment_timer(addr);
		return 0;
	}
}

std::vector<uint32_t> Memory::read_block(uint32_t addr) {
	clock++;
	if (query_timer(addr) == latency) {
		uint32_t index = decode_index(addr);
		uint32_t tag = decode_tag(addr);
		uint32_t offset = decode_offset(addr);
		Line *hitline = is_hit(index, tag);
		if (hitline) {
			sets[index].lru_incr(hitline);
			return hitline->read();
		}
		else {
			Line* lru = sets[index].find_LRU();
			if (lru->is_dirty()) {
				std::vector<uint32_t> old = lru->evict();
				uint32_t old_addr = encode_addr(lru->get_tag(), index, 0);
				Line * hit_line = next_level->is_hit(next_level->decode_index(old_addr), next_level->decode_tag(old_addr));
				if (hit_line)
					hit_line->write(old);
				else {
					while (next_level->query_timer(old_addr) < next_level->get_latency())
						next_level->write(old, old_addr);
					next_level->write(old, old_addr);
					next_level->reset_timer(old_addr);
				}
			}

			std::vector<uint32_t> new_block;
			while (next_level->query_timer(addr) < next_level->get_latency())
				new_block = next_level->read_block(addr);
			new_block = next_level->read_block(addr);
			next_level->reset_timer(addr);

			lru->write(new_block);
			lru->set_tag(tag);
			lru->set_dirty(false);
			lru->set_empty(false);
			sets[index].lru_incr(lru);
			std::cout << "Read complete." << std::endl;
			return new_block;
		}
	} else {
		std::cout << "Waiting..." << std::endl;
		increment_timer(addr);
		return std::vector<uint32_t>(0);
	}
}

void Memory::write(uint32_t word, uint32_t addr) {
	clock++;
	if (query_timer(addr) == latency) {
		uint32_t index = decode_index(addr);
		uint32_t tag = decode_tag(addr);
		uint32_t offset = decode_offset(addr);
		Line *hitline = is_hit(index, tag);
		if (hitline) {
			hitline->write(word, offset);
			sets[index].lru_incr(hitline);
		}
		else {
			Line* lru = sets[index].find_LRU();
			if (lru->is_dirty()) {
				std::vector<uint32_t> old = lru->evict();
				uint32_t old_addr = encode_addr(lru->get_tag(), index, 0);
				Line * hit_line = next_level->is_hit(next_level->decode_index(old_addr), next_level->decode_tag(old_addr));
				if (hit_line)
					hit_line->write(old);
				else {
					while (next_level->query_timer(old_addr) < next_level->get_latency())
						next_level->write(old, old_addr);
					next_level->write(old, old_addr);
					next_level->reset_timer(old_addr);
				}
			}

			std::vector<uint32_t> new_block;
			while (next_level->query_timer(addr) < next_level->get_latency())
				new_block = next_level->read_block(addr);
			new_block = next_level->read_block(addr);
			next_level->reset_timer(addr);

			lru->write(new_block);
			lru->write(word, offset);
			lru->set_tag(tag);
			lru->set_dirty(true);
			sets[index].lru_incr(lru);
			std::cout << "Write complete." << std::endl;
		}
	} else {
		std::cout << "Waiting..." << std::endl;
		increment_timer(addr);
	}
}

void Memory::write(std::vector<uint32_t> &block, uint32_t addr) {
	clock++;
	if (query_timer(addr) == latency) {
		uint32_t index = decode_index(addr);
		uint32_t tag = decode_tag(addr);
		uint32_t offset = decode_offset(addr);
		Line *hitline = is_hit(index, tag);
		if (hitline) {
			hitline->write(block);
			sets[index].lru_incr(hitline);
		}
		else {
			Line* lru = sets[index].find_LRU();
			if (lru->is_dirty()) {
				std::vector<uint32_t> old = lru->evict();
				uint32_t old_addr = encode_addr(lru->get_tag(), index, 0);
				Line * hit_line = next_level->is_hit(next_level->decode_index(old_addr), next_level->decode_tag(old_addr));
				if (hit_line)
					hit_line->write(old);
				else {
					while (next_level->query_timer(old_addr) < next_level->get_latency())
						next_level->write(old, old_addr);
					next_level->write(old, old_addr);
					next_level->reset_timer(old_addr);
				}
			}

			lru->write(block);
			lru->set_tag(tag);
			lru->set_dirty(true);
			sets[index].lru_incr(lru);
			std::cout << "Write complete." << std::endl;
		}
	} else {
		std::cout << "Waiting..." << std::endl;
		increment_timer(addr);
	}
}

uint32_t Memory::encode_addr(uint32_t tag, uint32_t index, uint32_t offset) {
	uint32_t addr = tag << index_n;
	addr |= index;
	addr <<= offset_n;
	addr |= offset;
	return addr;
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

Memory::Line* Memory::Set::is_hit(uint32_t tag) {
	Line* cur = 0;
	for (uint32_t i = 0; i < lines.size(); i++) {
		cur = &(lines[i]);
		if (cur->is_hit(tag) && !(cur->is_empty()))
			return cur;
	}
	return 0;
}

void Memory::Set::lru_incr(Line *recent_line)
{
	Line* cur = 0;
	for (uint32_t i = 0; i < lines.size(); i++) {
		cur = &(lines[i]);
		if (cur != recent_line)
			cur->age_incr();
	}
}

void Memory::Set::write(uint32_t word, uint32_t tag, uint32_t offset) {
	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			line.write(word, offset);
			line.age_reset();
		}
	}
}

void Memory::Set::write(std::vector<uint32_t> &block, uint32_t tag) {
	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			line.write(block);
			line.age_reset();
		}
	}
}

uint32_t Memory::Set::read(uint32_t tag, uint32_t offset) {
	uint32_t word = 0;

	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			word = line.read(offset);
			line.age_reset();
		}
	}

	return word;
}

std::vector<uint32_t> Memory::Set::read(uint32_t tag) {
	std::vector<uint32_t> block = std::vector<uint32_t>();

	for (Line line : lines) {
		if (line.is_hit(tag) && !line.is_empty()) {
			block = line.read();
			line.age_reset();
		}
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
	for (Line line : lines) {
		line.display();
	}
}

// Line methods

std::vector<uint32_t> Memory::Line::evict() {
	std::vector<uint32_t> temp = std::vector<uint32_t>(std::vector<uint32_t>(mem_array));
	empty = true;
	age_reset();
	return temp;
}

void Memory::Line::display() {
	std::cout << tag << " ";
	for (uint32_t i = 0; i < mem_array.size(); i++) {
		std::cout << std::hex << mem_array[i] << " ";
	}
	std::cout << std::endl;
}