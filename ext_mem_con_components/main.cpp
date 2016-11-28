#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <array>

const uint32_t ELEM_SIZE = 4;
const size_t BLOCK_SIZE = 1024;

class BufferedReader {
public:
	BufferedReader(FILE *input, size_t baseOffset, size_t elemsCount) :
		input(input), baseOffset(baseOffset), elemsCount(elemsCount) {}

	bool hasNext() {
		return readPos + 1 < elemsCount;
	}

	uint32_t read() {
		readPos++;
		if (readPos >= elemsCount)
			return -1;
		if (curBufferStart < 0 || readPos >= curBufferStart + BLOCK_SIZE) {
			curBufferStart = readPos;
			fseek(input, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
			fread(buffer.data(), ELEM_SIZE, std::min(BLOCK_SIZE, elemsCount - curBufferStart), input);
		}
		return buffer[readPos - curBufferStart];
	}

private:
	FILE *input;
	size_t baseOffset;
	size_t elemsCount;
	std::array<uint32_t, BLOCK_SIZE> buffer;

	int curBufferStart = -1;
	int readPos = -1;
};


class DSU {

public:
	DSU(uint32_t n) : n(n) {}
	~DSU() {
		delete[] p;
	}

	void Init() {
		srand(time(NULL));

		p = new uint32_t[n];
		for (uint32_t i = 0; i < n; ++i) {
			p[i] = i;
		}

		setsCount = n;
	}

	int Find(int x) {
		if (p[x] == x) return x;
		return p[x] = Find(p[x]);
	}

	void Union(int x, int y) {
		x = Find(x);
		y = Find(y);

		if (x == y)
			return;

		setsCount--;

		if (rand() % 2)
			std::swap(x, y);
		p[x] = y;
	}

	int SetsCount() {
		return setsCount;
	}

private:
	uint32_t n;

	uint32_t* p;
	uint32_t setsCount;
};

int main() {
	char* inputFileName = "input.bin";
	char* outputFileName = "output.bin";

	FILE *input = fopen(inputFileName, "rb");
	FILE *output = fopen(outputFileName, "wb");

	uint32_t n, m, result;

	fread(&n, 4, 1, input);
	fread(&m, 4, 1, input);

	BufferedReader* br = new BufferedReader(input, 8, 2 * m);

	DSU* dsu = new DSU(n);
	dsu->Init();

	while (br->hasNext()) {
		uint32_t a = br->read(), b = br->read();
		if (a != b)
			dsu->Union(a - 1, b - 1);
	}

	result = dsu->SetsCount();
	fwrite(&result, 4, 1, output);

	delete br;
	delete dsu;

	fclose(input);
	fclose(output);
}