#include <stdio.h>
#include <math.h>
#include <vector>
#include <stdlib.h>

typedef unsigned long long TICK_TYPE;

enum CacheReplacementPolicy { LRU, RANDOM };

class CacheLine
{
	size_t tag;
	TICK_TYPE touch_tick;
public:
	CacheLine() : tag(-1) {}
	void set_tag(size_t t) { tag = t; }
	size_t get_tag() { return tag; }
	void set_touch_tick(TICK_TYPE tt) { touch_tick = tt; }
	TICK_TYPE get_touch_tick() { return touch_tick; }
};

class CacheSet
{
	size_t cache_lines_count;
	const CacheReplacementPolicy replacement_policy;

	std::vector<CacheLine*> lines;

public:
	CacheSet(size_t cache_lines_count, CacheReplacementPolicy replacement_policy) :
		cache_lines_count(cache_lines_count),
		replacement_policy(replacement_policy)/*,
		cache_set_touch_tick(0)*/
	{
		lines.resize(cache_lines_count);
		for (size_t i = 0; i < cache_lines_count; i++)
			lines[i] = new CacheLine();
	}

	bool touch(size_t tag, TICK_TYPE touch_tick)
	{
		size_t empty_cache_line_index = -1;
		CacheLine* cache_line_with_min_tick = NULL;
		TICK_TYPE min_tick = touch_tick;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			CacheLine* cache_line = lines[i];
			if (cache_line->get_tag() == tag) {
				cache_line->set_touch_tick(touch_tick);
				return true;
			}
			if (cache_line->get_tag() == -1) {
				empty_cache_line_index = i;
			}
			if (min_tick > cache_line->get_touch_tick()) {
				min_tick = cache_line->get_touch_tick();
				cache_line_with_min_tick = cache_line;
			}
		}

		if (empty_cache_line_index != -1) {
			lines[empty_cache_line_index]->set_touch_tick(touch_tick);
			lines[empty_cache_line_index]->set_tag(tag);
			return false;
		}
		
		switch (replacement_policy) {
			case LRU:
				cache_line_with_min_tick->set_touch_tick(touch_tick);
				cache_line_with_min_tick->set_tag(tag);
				break;
			case RANDOM:
				int random_line_to_replace = rand() % lines.size();
				lines[random_line_to_replace]->set_touch_tick(touch_tick);
				lines[random_line_to_replace]->set_tag(tag);
				break;
		}

		return false;
	}
};

class Cache
{
	const size_t cache_size;
	const size_t cache_ways_count;
	const size_t cache_line_size;
	const CacheReplacementPolicy replacement_policy;
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
	Cache(size_t cache_size, size_t cache_ways_count, size_t cache_line_size, CacheReplacementPolicy replacement_policy = LRU, bool debug = false) :
		cache_size(cache_size),
		cache_ways_count(cache_ways_count),
		cache_line_size(cache_line_size),
		replacement_policy(replacement_policy),
		debug(debug)
	{
		cache_sets_count = cache_size / (cache_ways_count * cache_line_size);

		sets.resize(cache_sets_count);
		for (size_t i = 0; i < cache_sets_count; ++i)
			sets[i] = new CacheSet(cache_ways_count, replacement_policy);

		offset_in_block_size = static_cast<size_t>(log2(cache_line_size));
		index_size = static_cast<size_t>(log2(cache_sets_count));
		tag_size = 32 - offset_in_block_size - index_size;

		offset_in_block_mask = create_mask_of_ones(offset_in_block_size);
		index_mask = create_mask_of_ones(index_size);
		tag_mask = create_mask_of_ones(tag_size);

		touch_tick = 0;
		cache_hits = 0;

		// for generator used in CacheSets
		srand((unsigned int)time(NULL));
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
		if (debug)
			printf("Touch : %zu, %zu\n", index, tag);
		if (sets[index]->touch(tag, touch_tick)) {
			cache_hits++;
			if (debug)
				printf("Hit!\n");
		}
	}

	void print_cache_info()
	{
		printf("--- Cache info ---\n");
		printf("Cache size : %zu\n", cache_size);
		printf("Cache ways : %zu\n", cache_ways_count);
		printf("Cache line size : %zu\n", cache_line_size);
		printf("Cache sets count : %zu\n", cache_sets_count);
		printf("Offset size : %zu\n", offset_in_block_size);
		printf("Index size : %zu\n", index_size);
		printf("Tag size : %zu\n", tag_size);
		printf("Critical stride : %zu\n", cache_size / cache_ways_count);
	}

	void print_results()
	{
		printf("--- Cache state ---\n");
		printf("Cache touches : %llu\n", touch_tick);
		printf("Cache hits : %llu\n", cache_hits);
		printf("Cache misses : %llu\n", touch_tick - cache_hits);
		printf("Cache miss ratio : %.5f\n", (touch_tick - cache_hits) / (double) touch_tick);
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
