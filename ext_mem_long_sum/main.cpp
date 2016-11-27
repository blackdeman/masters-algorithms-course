#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <array>
#include <algorithm>
#include <iostream>

const int LENGTH_SIZE = 4;
const int ELEM_SIZE = 1;

const int BLOCK_SIZE = 100 * 1024; // 100 kb

const int ELEMS_PER_BLOCK = BLOCK_SIZE / ELEM_SIZE;

void printFile(char *fileName) {
	printf("File : %s\n", fileName);
	uint32_t N;
	uint8_t tmp;
	FILE *input = fopen(fileName, "rb");
	fread(&N, sizeof(N), 1, input);
	printf("Size : %d\n", N);
	for (size_t i = 0; i < N; ++i) {
		fread(&tmp, sizeof(tmp), 1, input);
		printf("%d ", tmp);
	}
	printf("\n");
}

class ReversedBufferedReader {
public:
	ReversedBufferedReader(FILE *input, size_t baseOffset, size_t elemsCount) :
		input(input), baseOffset(baseOffset), elemsCount(elemsCount), readPos(elemsCount) {}

	bool hasNext() {
		return readPos > 0;
	}

	uint8_t read() {
		readPos--;
		if (readPos < 0) {
			printf("Error reading!");
			return -1;
		}
		if (curBufferStart < 0 || readPos < curBufferStart) {
			curBufferStart = std::max(readPos - ELEMS_PER_BLOCK + 1, 0);
			fseek(input, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
			fread(buffer.data(), ELEM_SIZE, std::min((int)ELEMS_PER_BLOCK, readPos + 1), input);
		}
		return buffer[readPos - curBufferStart];
	}

private:
	FILE *input;
	size_t baseOffset;
	size_t elemsCount;
	std::array<uint8_t, ELEMS_PER_BLOCK> buffer;

	int curBufferStart = -1;
	int readPos;
};

class BufferedWriter {
public:
	BufferedWriter(FILE *output, size_t baseOffset) :
		output(output), baseOffset(baseOffset) {}

	void write(uint8_t value) {
		writePos++;
		if (writePos >= curBufferStart + BLOCK_SIZE) {
			fwrite(buffer.data(), ELEM_SIZE, writePos - curBufferStart, output);
			curBufferStart += BLOCK_SIZE;
		}
		buffer[writePos - curBufferStart] = value;
	}

	void flush() {
		fwrite(buffer.data(), ELEM_SIZE, writePos + 1 - curBufferStart, output);
	}

private:
	FILE *output;
	size_t baseOffset;
	std::array<uint8_t, BLOCK_SIZE> buffer;

	int curBufferStart = 0;
	int writePos = -1;
};

int main() {
	char* inputFileName = "input-8-3.bin";
	char* tmpFileName = "tmp.bin";
	char* outputFileName = "output.bin";

	FILE *input = fopen(inputFileName, "rb");
	FILE *tmp = fopen(tmpFileName, "wb");

	uint32_t n, m, k = 0;

	fread(&n, LENGTH_SIZE, 1, input);
	fseek(input, n * ELEM_SIZE, SEEK_CUR);
	fread(&m, sizeof(m), 1, input);

	ReversedBufferedReader* aBr = new ReversedBufferedReader(input, LENGTH_SIZE, n);
	ReversedBufferedReader* bBr = new ReversedBufferedReader(input, LENGTH_SIZE + n * ELEM_SIZE + LENGTH_SIZE, m);

	BufferedWriter* cBw = new BufferedWriter(tmp, LENGTH_SIZE);

	uint8_t ai, bi, ci, carry = 0;
	while (aBr->hasNext() || bBr->hasNext()) {
		k++;

		if (aBr->hasNext())
			ai = aBr->read();
		else
			ai = 0;
		
		if (bBr->hasNext())
			bi = bBr->read();
		else
			bi = 0;

		ci = ai + bi + carry;
		carry = ci / 10;
		ci %= 10;

		cBw->write(ci);
	}
	
	if (carry) {
		k++;
		cBw->write(carry);
	}

	cBw->flush();

	fclose(input);
	fclose(tmp);

	delete aBr;
	delete bBr;
	delete cBw;

	tmp = fopen(tmpFileName, "rb");
	FILE *output = fopen(outputFileName, "wb");

	ReversedBufferedReader* revResultBr = new ReversedBufferedReader(tmp, 0, k);
	BufferedWriter* resultBw = new BufferedWriter(output, LENGTH_SIZE);

	fwrite(&k, LENGTH_SIZE, 1, output);
	while (revResultBr->hasNext()) {
		resultBw->write(revResultBr->read());
	}
	resultBw->flush();

	fclose(tmp);
	fclose(output);

	delete revResultBr;
	delete resultBw;

	printFile("output.bin");
}