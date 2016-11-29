#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <array>

const unsigned int BLOCK_SIZE = 100 * 1024;
const unsigned int ELEM_SIZE = 1;
const unsigned int SIZE_OFFSET = 2 * sizeof(uint32_t);

class BufferedReader {
public:
	BufferedReader(FILE *input, size_t baseOffset, size_t elemsCount) :
		input(input), baseOffset(baseOffset), elemsCount(elemsCount) {}

	bool hasNext() {
		return readPos + 1 < elemsCount;
	}

	uint8_t read() {
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

	void reset() {
		readPos = -1;
		if (curBufferStart != 0)
			curBufferStart = -1;
	}

private:
	FILE *input;
	size_t baseOffset;
	size_t elemsCount;
	std::array<uint8_t, BLOCK_SIZE> buffer;

	int curBufferStart = -1;
	int readPos = -1;
};

class BufferedWriter {
public:
	BufferedWriter(FILE *output, size_t baseOffset) :
		output(output), baseOffset(baseOffset) {}

	void write(uint8_t value) {
		writePos++;
		if (writePos >= curBufferStart + BLOCK_SIZE) {
			//fseek(output, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
			fwrite(buffer.data(), ELEM_SIZE, writePos - curBufferStart, output);
			curBufferStart += BLOCK_SIZE;
		}
		buffer[writePos - curBufferStart] = value;
	}

	void flush() {
		//fseek(output, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
		fwrite(buffer.data(), ELEM_SIZE, writePos + 1 - curBufferStart, output);
	}

private:
	FILE *output;
	size_t baseOffset;
	std::array<uint8_t, BLOCK_SIZE> buffer;

	int curBufferStart = 0;
	int writePos = -1;
};

void print_matrix_from_file(const char* path) {
	printf("Print matrix\n");
	FILE* in = fopen(path, "rb");

	uint32_t rows = 3;
	uint32_t cols = 1;

	printf("%d %d\n", rows, cols);

	uint8_t el = 0;
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			fread(&el, sizeof(el), 1, in);
			printf("%3d ", (int)el);
		}
		printf("\n");
	}
	printf("\n");

	fclose(in);
}

int main() {
	FILE* input = fopen("input.bin", "rb");
	FILE* output = fopen("output.bin", "wb");

	uint32_t N, M;

	fread(&N, sizeof(N), 1, input);
	fread(&M, sizeof(M), 1, input);

	BufferedReader* aBr = new BufferedReader(input, SIZE_OFFSET, N * M);
	BufferedReader* bBr = new BufferedReader(input, SIZE_OFFSET + N * M, M);

	BufferedWriter* cBw = new BufferedWriter(output, 0);

	for (size_t i = 0; i < N; ++i) {
		uint8_t ci = 0;
		for (size_t j = 0; j < M; ++j) {
			ci += aBr->read() * bBr->read() % 256;
		}
		bBr->reset();
		cBw->write(ci);
	}
	cBw->flush();

	delete aBr;
	delete bBr;
	delete cBw;

	fclose(input);
	fclose(output);

	return 0;
}