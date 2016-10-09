#include <iostream>
#include <random>
#include <ctime>

#include "Cache.h"

//__declspec(align(16))
// __restrict
namespace {
	void MultSimple(const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
	{
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				c[i * n + j] = 0.f;
				for (int k = 0; k < n; ++k) {
					c[i * n + j] += a[i * n + k] + b[k * n + j];
				}
			}
		}
	}

	void MultSimpleAnalyzed(Cache& cache, const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
    {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cache.write(&c[i * n + j], 0.f);
                for (int k = 0; k < n; ++k) {
					cache.write(&c[i * n + j], cache.read(&c[i * n + j]) + cache.read(&a[i * n + k]) * cache.read(&b[k * n + j]));
                }
            }
        }
    }
    
    void FillRandom(float* a, int n)
    {
        std::default_random_engine eng;
        std::uniform_real_distribution<float> dist;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = dist(eng);
            }
        }
    }

	void PrintMatrix(const float* a, int n)
	{
		printf("\n");
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				printf("%.3f ", a[i * n + j]);
			}
			printf("\n");
		}
	}
}

int main(int argc, char* argv[])
{
	const int n = 1024;//atoi(argv[1]);
    std::cerr << "n = " << n << std::endl;

	const size_t cache_size = 8 * 1024;//3145728;
	const size_t cache_ways_count = 4;//12;
	const size_t cache_line_size = 64;//64;

	const bool debug = false;
    
	Cache cache(cache_size, cache_ways_count, cache_line_size, debug);
	cache.print_cache_info();

    float* a = new float[n * n];
    float* b = new float[n * n];
    float* c = new float[n * n];

    FillRandom(a, n);
    FillRandom(b, n);

    {
        const auto startTime = std::clock();
        // MultSimple(a, b, c, n);
		MultSimpleAnalyzed(cache, a, b, c, n);
        const auto endTime = std::clock();

        // std::cerr << "timeSimple: " << double(endTime - startTime) / CLOCKS_PER_SEC << '\n';
		
		if (debug) {
			PrintMatrix(a, n);
			PrintMatrix(b, n);
			PrintMatrix(c, n);
		}
	}

	cache.print_results();
    
    delete[] a;
    delete[] b;
    delete[] c;
}

