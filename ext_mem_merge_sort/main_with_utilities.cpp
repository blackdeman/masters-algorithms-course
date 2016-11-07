#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <array>
#include <queue>
#include <functional>

//#include <ctime>

const size_t BLOCK_SIZE = 512;
const uint64_t MEM_SIZE = 300 * 1024;

const uint64_t ELEM_SIZE = 8;
const uint64_t ELEMS_PER_BLOCK = BLOCK_SIZE / ELEM_SIZE;
const uint64_t ELEMS_PER_MEM = MEM_SIZE / ELEM_SIZE;

const uint64_t MERGE_WAYS = MEM_SIZE / BLOCK_SIZE;

bool test(char* inputFileName, char* outputFileName) {
	uint64_t N, N2, tmp;
	FILE *input = fopen(inputFileName, "rb");
	FILE *output = fopen(outputFileName, "rb");
	fread(&N, sizeof(N), 1, input);
	fread(&N2, sizeof(N2), 1, output);

	if (N != N2) {
		return false;
	}

	std::vector<uint64_t> buffer;
	buffer.resize(N);
	fread(buffer.data(), ELEM_SIZE, N, input);
	std::sort(buffer.begin(), buffer.end());

	for (size_t i = 0; i < N; ++i) {
		fread(&tmp, ELEM_SIZE, 1, output);
		if (tmp != buffer[i])
			return false;
	}
	return true;
}


void printFile(char *fileName) {
	printf("File : %s\n", fileName);
	uint64_t N, tmp;
	FILE *input = fopen(fileName, "rb");
	fread(&N, sizeof(N), 1, input);
	printf("Size : %lld\n", N);
	for (size_t i = 0; i < N; ++i) {
		fread(&tmp, sizeof(tmp), 1, input);
		printf("%lld ", tmp);
	}
	printf("\n");
}

class BufferedReader {
public:
	BufferedReader(FILE *input, size_t baseOffset, size_t elemsCount) :
		input(input), baseOffset(baseOffset), elemsCount(elemsCount) {}

	bool hasNext() {
		return readPos + 1 < elemsCount;
	}

	uint64_t read() {
		readPos++;
		if (readPos >= elemsCount)
			return -1;
		if (curBufferStart < 0 || readPos >= curBufferStart + BLOCK_SIZE) {
			curBufferStart = readPos;
			fseek(input, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
			fread(buffer.data(), ELEM_SIZE, std::min(BLOCK_SIZE, elemsCount - curBufferStart), input);
		}
		//printf("return %d, %d, elems : %d", readPos, curBufferStart, elemsCount);
		return buffer[readPos - curBufferStart];
	}

private:
	FILE *input;
	size_t baseOffset;
	size_t elemsCount;
	std::array<uint64_t, BLOCK_SIZE> buffer;

	int curBufferStart = -1;
	int readPos = -1;
};

class BufferedWriter {
public:
	BufferedWriter(FILE *output, size_t baseOffset) :
		output(output), baseOffset(baseOffset) {}

	void write(uint64_t value) {
		writePos++;
		if (writePos >= curBufferStart + BLOCK_SIZE) {
			//flush();
			fwrite(buffer.data(), 8, writePos - curBufferStart, output);
			curBufferStart += BLOCK_SIZE;
		}
		buffer[writePos - curBufferStart] = value;
	}

	void flush() {
		//fseek(output, baseOffset + curBufferStart * ELEM_SIZE, SEEK_SET);
		fwrite(buffer.data(), 8, writePos + 1 - curBufferStart, output);
	}

private:
	FILE *output;
	size_t baseOffset;
	std::array<uint64_t, BLOCK_SIZE> buffer;

	int curBufferStart = 0;
	int writePos = -1;
};

typedef std::pair<uint64_t, size_t> Pair;

struct PairCmp {
	bool operator() (const Pair p1, const Pair p2) const {
		return p1.first > p2.first;
	}
};

void sortParts(uint64_t* parts, size_t partsCount, uint64_t n, FILE *input, int inputBaseOffset, char* outputFileName, int outputBaseOffset) {
	FILE *output = fopen(outputFileName, "wb");
	fwrite(&n, ELEM_SIZE, 1, output);
	
	std::array<uint64_t, ELEMS_PER_MEM> buffer;

	for (size_t i = 0; i < partsCount; ++i) {
		//int bufferSize = std::min(ELEMS_PER_MEM, n - i * ELEMS_PER_MEM);
		size_t bufferSize = parts[i + 1] - parts[i];
		//printf("Sorting elements from %lld to %lld\n", parts[i], parts[i] + bufferSize);
		fread(buffer.data(), ELEM_SIZE, bufferSize, input);
		std::sort(buffer.begin(), buffer.begin() + bufferSize);
		fwrite(buffer.data(), ELEM_SIZE, bufferSize, output);
	}

	fclose(output);
}

void mergeParts(uint64_t* parts, size_t &partsCount, uint64_t n, char *inputFileName, int inputBaseOffset, char *outputFileName, int outputBaseOffset) {
	FILE *input = fopen(inputFileName, "rb");
	FILE *output = fopen(outputFileName, "wb");

	fwrite(&n, ELEM_SIZE, 1, output);

	size_t mergeIters = partsCount / MERGE_WAYS;
	if (partsCount % MERGE_WAYS) mergeIters++;

	std::vector<BufferedReader*> readers;
	parts[0] = 0;
	size_t i = 0;
	for (i = 0; i < mergeIters; ++i) {
		size_t startPart = i * partsCount / mergeIters;
		size_t endPart = std::min((i + 1) * partsCount / mergeIters, partsCount);
		//printf("Merge iter %d\n", i);
		//printf("Merging parts from %d to %d\n", startPart, endPart);
		
		std::priority_queue <Pair, std::vector<Pair>, PairCmp> queue;
		for (size_t j = startPart; j < endPart; ++j) {
			BufferedReader *br = new BufferedReader(input, inputBaseOffset + parts[j] * ELEM_SIZE, parts[j + 1] - parts[j]);
			uint64_t value = br->read();
			queue.push(Pair(value, readers.size()));
			readers.push_back(br);
		}
		
		BufferedWriter * bw = new BufferedWriter(output, outputBaseOffset);

		while (!queue.empty()) {
			Pair p = queue.top();
			queue.pop();
			bw->write(p.first);
			if (readers[p.second]->hasNext()) {
				queue.push(Pair(readers[p.second]->read(), p.second));
			}
		}
		bw->flush();

		for (int j = 0; j < readers.size(); ++j)
			delete readers[j];
		delete bw;
		readers.clear();
		parts[i + 1] = parts[endPart];
	}

	partsCount = mergeIters;

	fclose(input);
	fclose(output);
}

int main() {
	//long start = clock_t();
	char *inputFileName = "input.bin";
	char *outputFileName = "output.bin";
	char *tmpFileName1 = "tmp_file_1.bin";
	char *tmpFileName2 = "tmp_file_2.bin";

	uint64_t N;

	FILE *input = fopen(inputFileName, "rb");

	fread(&N, sizeof(N), 1, input);
	
	size_t partsCount = N / ELEMS_PER_MEM;
	if (N % ELEMS_PER_MEM) partsCount++;
	uint64_t *parts = new uint64_t[partsCount + 1];
	for (size_t i = 0; i < partsCount; ++i) {
		parts[i] = i * N / partsCount;
	}
	parts[partsCount] = N;

	sortParts(parts, partsCount, N, input, ELEM_SIZE, tmpFileName2, ELEM_SIZE);

	fclose(input);

	//for (int i = 0; i < partsCount + 1; ++i) {
	//	printf("%d ", parts[i]);
	//}
	//printf("\n");
	
	while (partsCount > 1) {
		//printf("--- Merge ---\n");
		std::swap(tmpFileName1, tmpFileName2);
		mergeParts(parts, partsCount, N, tmpFileName1, ELEM_SIZE, tmpFileName2, ELEM_SIZE);

		//for (size_t i = 0; i < partsCount + 1; ++i) {
		//	printf("%d ", parts[i]);
		//}
		//printf("\n");
	}

	std::remove(outputFileName);
	std::rename(tmpFileName2, outputFileName);
	/*FILE *in = fopen(tmpFileName2, "rb");
	FILE *out = fopen(outputFileName, "wb");

	for (int i = 0; i < N; ++i) {
		uint64_t a;
		fread(&a, ELEM_SIZE, 1, in);
		fwrite(&a, ELEM_SIZE, 1, out);
	}

	fclose(in);
	fclose(out);*/

	//printFile(inputFileName);
	//printFile(outputFileName);
	/*printf("Finish : %.5f\n", (clock() - start) / (double)CLOCKS_PER_SEC);
	if (test(inputFileName, outputFileName)) {
		printf("Ok!\n");
	}
	else {
		printf("Fail...\n");
	}*/
}