#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>

#include <string>
#include <sstream>
#include <bitset>

typedef unsigned long long TICK_TYPE;

class CacheLine
{
	size_t tag;
	TICK_TYPE touch_tick;
public:
	CacheLine() : tag(-1) {}
	void set_tag(size_t t) { tag = t; }
	size_t get_tag() { return tag; }
	void set_touch_tick(unsigned long long tt) { touch_tick = tt; }
	unsigned long long get_touch_tick() { return touch_tick; }
};

class CacheSet
{
	size_t cache_lines_count;

	std::vector<CacheLine*> lines;

public:
	CacheSet(size_t cache_lines_count) :
		cache_lines_count(cache_lines_count)
	{
		lines.resize(cache_lines_count);
		std::fill(lines.begin(), lines.end(), new CacheLine());
	}

	bool touch(size_t tag, TICK_TYPE touch_tick)
	{
		CacheLine* cache_line_with_min_tick = NULL;
		TICK_TYPE min_tick = touch_tick;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			CacheLine* cache_line = lines[i];
			if (cache_line->get_tag() == tag) {
				cache_line->set_touch_tick(touch_tick);
				return true;
			}
			else if (cache_line->get_tag() == -1) {
				cache_line->set_touch_tick(touch_tick);
				cache_line->set_tag(tag);
				return false;
			}
			if (min_tick > cache_line->get_touch_tick()) {
				min_tick = cache_line->get_touch_tick();
				cache_line_with_min_tick = cache_line;
			}
		}

		cache_line_with_min_tick->set_touch_tick(touch_tick);
		cache_line_with_min_tick->set_tag(tag);
	
		return false;
	}
};

class Cache
{
	const size_t cache_size;
	const size_t cache_ways_count;
	const size_t cache_line_size;
	const bool debug;

	size_t cache_sets_count;
	std::vector<CacheSet*> sets;
	size_t offset_in_block_size;
	size_t index_size;
	size_t tag_size;

	size_t offset_in_block_mask;
	size_t index_mask;
	size_t tag_mask;

	TICK_TYPE touch_tick;
	TICK_TYPE cache_hits;

public:
	Cache(size_t cache_size, size_t cache_ways_count, size_t cache_line_size, bool debug) :
		cache_size(cache_size),
		cache_ways_count(cache_ways_count),
		cache_line_size(cache_line_size),
		debug(debug)
	{
		cache_sets_count = cache_size / (cache_ways_count * cache_line_size);

		sets.resize(cache_sets_count);
		std::fill(sets.begin(), sets.end(), new CacheSet(cache_ways_count));

		offset_in_block_size = static_cast<size_t>(log2(cache_line_size));
		index_size = static_cast<size_t>(log2(cache_sets_count));
		tag_size = 32 - offset_in_block_size - index_size;

		offset_in_block_mask = create_mask_of_ones(offset_in_block_size);
		index_mask = create_mask_of_ones(index_size);
		tag_mask = create_mask_of_ones(tag_size);

		touch_tick = 0;
		cache_hits = 0;
		/*
		float x = 10;
		float* ptr = &x;

		printf("%p\n", ptr);
		printf("%d\n", (size_t)ptr);
		
		char* str = new char[9];// = "0x31c3";
		sprintf_s(str, 9, "%p", ptr);
		std::istringstream iss(str);
		int i;
		iss >> std::hex >> i;
		if (!iss && !iss.eof()) throw "dammit!";
		std::cout << '"' << str << "\": " << i << "(0x" << std::hex << i << ")\n";
		std::bitset<32> bs = i;
		std::cout << "Binary : " << bs.to_string<char, std::char_traits<char>, std::allocator<char> >() << std::endl;
		
		std::cout << block_for_ptr(ptr) << std::endl;
		std::cout << index_for_ptr(ptr) << std::endl;
		std::cout << tag_for_ptr(ptr) << std::endl;
		printf("%d\n", tag_for_ptr(ptr));

		printf("%d\n", (tag_for_ptr(ptr) << (offset_in_block_size + index_size)) + (index_for_ptr(ptr) << offset_in_block_size) + block_for_ptr(ptr));
		*/
	}

	int read(const int* ptr)
	{
		if (debug)
			printf("Reading from %p\n", ptr);
		touch(ptr);
		return *ptr;
	}

	float read(const float* ptr)
	{
		if (debug)
			printf("Reading from %p\n", ptr);
		touch(ptr);
		return *ptr;
	}

	void write(float* ptr, float value)
	{
		if (debug)
			printf("Writing to %p\n", ptr);
		touch(ptr);
		*ptr = value;
	}

	void touch(const void* ptr)
	{
		touch_tick++;

		size_t index = index_for_ptr(ptr);
		size_t tag = tag_for_ptr(ptr);

		if (sets[index]->touch(tag, touch_tick)) {
			cache_hits++;
		}
	}

	void print_cache_info()
	{
		printf("--- Cache info ---\n");
		printf("Cache size : %d\n", cache_size);
		printf("Cache ways : %d\n", cache_ways_count);
		printf("Cache line size : %d\n", cache_line_size);
		printf("Cache sets count : %d\n", cache_sets_count);
		printf("Offset size : %d\n", offset_in_block_size);
		printf("Index size : %d\n", index_size);
		printf("Tag size : %d\n", tag_size);
		//printf("Critical stride : %d\n", cache_size / cache_ways_count);
	}

	void print_results()
	{
		printf("--- Cache state ---\n");
		printf("Cache touches : %llu\n", touch_tick);
		printf("Cache hits : %llu\n", cache_hits);
		printf("Cache misses : %llu\n", touch_tick - cache_hits);
		printf("Cache miss ratio : %.3f\n", (touch_tick - cache_hits) / (double) touch_tick);
	}

private:
	size_t tag_for_ptr(const void* ptr) {
		return ((size_t)ptr >> (offset_in_block_size + index_size)) & tag_mask;
	}
	
	size_t index_for_ptr(const void* ptr) {
		return ((size_t)ptr >> offset_in_block_size) & index_mask;
	}

	size_t block_for_ptr(const void* ptr) {
		return (size_t)ptr & offset_in_block_mask;
	}

	inline size_t create_mask_of_ones(size_t ones_count) {
		return (1ull << ones_count) - 1ull;
	}
	
};