#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>

void printFile(char *fileName) {
	printf("File : %s\n", fileName);
	uint32_t N, tmp;
	FILE *input = fopen(fileName, "rb");
	fread(&N, sizeof(N), 1, input);
	printf("Value : %d\n", N);
	fclose(input);
}

int main() {
	FILE *input = fopen("input.bin", "rb");
	FILE *output = fopen("output.bin", "wb");

	uint32_t a, b, c;

	fread(&a, sizeof(a), 1, input);
	fread(&b, sizeof(b), 1, input);

	c = a + b;

	fwrite(&c, sizeof(c), 1, output);

	fclose(input);
	fclose(output);

	printFile("output.bin");
}