#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <array>

const size_t BLOCK_SIZE = 512;

class BufferedWriter {
public:
	BufferedWriter(FILE *output, size_t baseOffset) :
		output(output), baseOffset(baseOffset) {}

	void write(uint64_t value) {
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

	BufferedWriter * bw = new BufferedWriter(output, 0);
	for (uint32_t i = 1; i <= N; ++i)
		bw->write(i);

	bw->flush();

	fclose(input);
	fclose(output);
}
