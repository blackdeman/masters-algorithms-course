#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <array>

const size_t BLOCK_SIZE = 512;

class BufferedWriter {
public:
	BufferedWriter(FILE *output, size_t baseOffset) :
		output(output), baseOffset(baseOffset) {}

	void write(uint32_t value) {
		writePos++;
		if (writePos >= curBufferStart + BLOCK_SIZE) {
			fwrite(buffer.data(), 4, writePos - curBufferStart, output);
			curBufferStart += BLOCK_SIZE;
		}
		buffer[writePos - curBufferStart] = value;
	}

	void flush() {
		fwrite(buffer.data(), 4, writePos + 1 - curBufferStart, output);
	}

private:
	FILE *output;
	size_t baseOffset;
	std::array<uint32_t, BLOCK_SIZE> buffer;

	int curBufferStart = 0;
	int writePos = -1;
};

int main() {
	char *inputFileName = "input.bin";
	char *outputFileName = "output.bin";

	uint32_t N;

	FILE *input = fopen(inputFileName, "rb");
	FILE *output = fopen(outputFileName, "wb");

	fread(&N, sizeof(N), 1, input);

	uint32_t* data = new uint32_t[2 * N];

	fread(data, sizeof(uint32_t), 2 * N, input);
	
	BufferedWriter * bw = new BufferedWriter(output, 0);

	uint32_t firstElIndex = 0;
	uint32_t firstEl = data[firstElIndex];
	for (uint32_t i = 0; i < N; ++i) {
		if (data[2 * i] < firstEl) {
			firstElIndex = 2 * i;
			firstEl = data[2 * i];
		}
	}

	bw->write(firstEl);
	uint32_t curEl = data[firstElIndex + 1];

	while (curEl != firstEl) {
		bw->write(curEl);
		
		for (uint32_t i = 0; i < N; ++i) {
			if (curEl == data[2 * i]) {
				curEl = data[2 * i + 1];
				break;
			}
		}
	};

	bw->flush();

	fclose(input);
	fclose(output);

	delete[] data;
}
