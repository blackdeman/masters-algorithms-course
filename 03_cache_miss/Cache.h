#include <stdio.h>
#include <cmath>
#include <vector>
#include <algorithm>

class CacheLine
{
	size_t tag;
	unsigned long long touch_tick;
public:
	CacheLine() : tag(NULL) {}
	void set_tag(size_t t) { tag = t; }
	size_t get_tag() { return tag; }
	void set_touch_tick(unsigned long long tt) { touch_tick = tt; }
	unsigned long long get_touch_tick() { return touch_tick; }
};

class CacheSet
{
	size_t cache_line_size;
	size_t cache_lines_count;

	std::vector<CacheLine*> lines;

public:
	CacheSet(size_t cache_lines_count, size_t cache_line_size) :
		cache_lines_count(cache_lines_count),
		cache_line_size(cache_line_size)
	{
		lines.resize(cache_lines_count);
		std::fill(lines.begin(), lines.end(), new CacheLine());
	}

	CacheLine* get_line(size_t line_number) {
		return lines[line_number];
	}
};

class Cache
{
	const size_t cache_size;
	const size_t cache_ways_count;
	const size_t cache_line_size;
	const bool debug;

	size_t cache_lines_per_set;
	std::vector<CacheSet*> sets;
	size_t offset_in_block_size;
	size_t index_size;
	size_t tag_size;

	size_t offset_in_block_mask;
	size_t index_mask;
	size_t tag_mask;

	unsigned long long touch_tick;
	unsigned long long cache_hits;

public:
	Cache(size_t cache_size, size_t cache_ways_count, size_t cache_line_size, bool debug) :
		cache_size(cache_size),
		cache_ways_count(cache_ways_count),
		cache_line_size(cache_line_size),
		debug(debug) 
	{
		cache_lines_per_set = cache_size / (cache_ways_count * cache_line_size);

		sets.resize(cache_ways_count);
		std::fill(sets.begin(), sets.end(), new CacheSet(cache_lines_per_set, cache_line_size));

		offset_in_block_size = static_cast<size_t>(log2(cache_line_size));
		index_size = static_cast<size_t>(log2(cache_lines_per_set));
		tag_size = 32 - offset_in_block_size - index_size;

		offset_in_block_mask = create_mask_of_ones(offset_in_block_size);
		index_mask = create_mask_of_ones(index_size);
		tag_mask = create_mask_of_ones(tag_size);

		touch_tick = 0;
		cache_hits = 0;
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

	void print_cache_info()
	{
		printf("--- Cache info ---\n");
		printf("Cache size : %d\n", cache_size);
		printf("Cache ways : %d\n", cache_ways_count);
		printf("Cache line size : %d\n", cache_line_size);
		printf("Cache lines per set : %d\n", cache_lines_per_set);
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
	void touch(const void* ptr)
	{
		touch_tick++;

		size_t index = index_for_ptr(ptr);
		size_t tag = tag_for_ptr(ptr);

		bool processed = false;
		CacheLine* cache_line_with_min_tick = NULL;
		unsigned long long min_tick = NULL;

		for (size_t i = 0; i < sets.size(); ++i)
		{
			CacheLine* cache_line = sets[i]->get_line(index);
			if (cache_line->get_tag() == tag) {
				cache_line->set_touch_tick(touch_tick);
				cache_hits++;
				processed = true;
				break;
			}
			else if (cache_line->get_tag() == NULL) {
				cache_line->set_touch_tick(touch_tick);
				cache_line->set_tag(tag);
				processed = true;
				break;
			}
			if (min_tick == NULL || min_tick < cache_line->get_touch_tick()) {
				min_tick = cache_line->get_touch_tick();
				cache_line_with_min_tick = cache_line;
			}
		}

		if (!processed) {
			cache_line_with_min_tick->set_touch_tick(touch_tick);
			cache_line_with_min_tick->set_tag(tag);
		}

	}

	size_t tag_for_ptr(const void* ptr) {
		return ((size_t)ptr >> (offset_in_block_size + index_size)) & tag_size;
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