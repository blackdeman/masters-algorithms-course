#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>

int main() {
	char *inputFileName = "input.bin";
	char *outputFileName = "output.bin";

	uint32_t N;

	FILE *input = fopen(inputFileName, "rb");
	FILE *output = fopen(outputFileName, "wb");

	fread(&N, sizeof(N), 1, input);

	uint32_t* data = new uint32_t[2 * N];

	fread(data, sizeof(uint32_t), 2 * N, input);
	
	fwrite(data, sizeof(uint32_t), N, output);

	fclose(input);
	fclose(output);

	delete[] data;
}
