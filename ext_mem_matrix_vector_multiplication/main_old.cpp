#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdint>
#include <algorithm>

const unsigned int BLOCK = 400;
const unsigned int SIZE_OFFSET = 2 * sizeof(uint32_t);

void read_block(FILE* input, size_t stream_offset, 
	uint8_t *buffer,
	size_t upper_row, size_t buffer_height, size_t left_col, size_t buffer_width,
	size_t row_count, size_t col_count) {

	size_t upper_left_offset_read = stream_offset + upper_row * col_count + left_col;
	if (buffer_width == col_count) {
		fseek(input, upper_left_offset_read, SEEK_SET);
		fread(buffer, buffer_width * buffer_height, 1, input);
	}
	else {
		for (size_t i = 0; i < buffer_height; ++i){
			fseek(input, upper_left_offset_read + i * col_count, SEEK_SET);
			fread(buffer + buffer_width * i, buffer_width, 1, input);
		}
	}
}

void write_block(FILE* output, size_t stream_offset,
	uint8_t *buffer,
	size_t upper_row, size_t buffer_height, size_t left_col, size_t buffer_width,
	size_t row_count, size_t col_count) {

	size_t upper_left_offset_write = stream_offset + upper_row * col_count + left_col;
	fseek(output, upper_left_offset_write, SEEK_SET);
	
	if (buffer_width == col_count) {
		fseek(output, upper_left_offset_write, SEEK_SET);
		fwrite(buffer, buffer_width * buffer_height, 1, output);
	}
	else {
		for (size_t i = 0; i < buffer_height; ++i){
			fseek(output, upper_left_offset_write + i * col_count, SEEK_SET);
			fwrite(buffer + buffer_width * i, buffer_width, 1, output);
		}
	}
}

void print_matrix_from_file(const char* path) {
	printf("Print matrix\n");
	FILE* in = fopen(path, "rb");

	uint32_t rows = 3;
	uint32_t cols = 1;

	//in.read((char*)&rows, sizeof(rows));
	//in.read((char*)&cols, sizeof(cols));

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

void print_buffer(uint8_t *buffer) {
	for (size_t i = 0; i < BLOCK; ++i) {
		for (size_t j = 0; j < BLOCK; ++j) {
			printf("%3d ", buffer[i * BLOCK + j]);
		}
		printf("\n");
	}
	printf("\n");
}

int main() {
	FILE* input = fopen("input.bin", "rb");
	FILE* output = fopen("output.bin", "wb");

	uint32_t N, M, K = 1;
	
	fread(&N, sizeof(N), 1, input);
	fread(&M, sizeof(M), 1, input);

	uint8_t *left_buffer = new uint8_t[BLOCK * BLOCK];
	uint8_t *right_buffer = new uint8_t[BLOCK * BLOCK];
	uint8_t *result_buffer = new uint8_t[BLOCK * BLOCK];

	size_t left_matrix_offset = SIZE_OFFSET;
	size_t right_matrix_offset = SIZE_OFFSET + N * M;
	size_t result_matrix_offset = 0;

	// Calculate suitable blocks to process
	size_t result_row_step = 0;
	size_t result_col_step = 1;
	size_t common_step = 0;

	if (N < BLOCK) {
		if (M < BLOCK) {
			result_row_step = N;
			common_step = M;
		}
		else {
			result_row_step = N;
			common_step = BLOCK * BLOCK / N;
		}
	}
	else {
		if (M < BLOCK) {
			result_row_step = BLOCK * BLOCK / M;
			common_step = M;
		}
		else {
			result_row_step = BLOCK;
			common_step = BLOCK;
		}
	}

	for (size_t upper_row_result = 0; upper_row_result < N; upper_row_result += result_row_step) {
		size_t result_height = std::min(result_row_step, N - upper_row_result);
		for (size_t left_col_result = 0; left_col_result < K; left_col_result += result_col_step) {
			size_t result_width = std::min(result_col_step, K - left_col_result);
			
			size_t left_matrix_height = result_height;
			size_t right_matrix_width = result_width;

			for (size_t k = 0; k < M; k += common_step) {
				size_t left_matrix_width = std::min(common_step, M - k);
				size_t right_matrix_height = left_matrix_width;

				// read left matrix part
				read_block(input, left_matrix_offset, left_buffer, upper_row_result, left_matrix_height, k, left_matrix_width, N, M);

				// read right matrix part
				read_block(input, right_matrix_offset, right_buffer, k, right_matrix_height, left_col_result, right_matrix_width, M, K);
				
				// calculus
				for (size_t ib = 0; ib < result_height; ++ib) {
					for (size_t jb = 0; jb < result_width; ++jb) {
						if (k == 0)
							result_buffer[ib * result_width + jb] = 0;
						for (size_t kb = 0; kb < left_matrix_width/*right_matrix_row_count*/; ++kb) {
							result_buffer[ib * result_width + jb] += 
								left_buffer[ib * left_matrix_width + kb] * right_buffer[kb * right_matrix_width + jb];
						}
					}
				}

			}
			// write result matrix part
			write_block(output, result_matrix_offset, result_buffer, upper_row_result, result_height, left_col_result, result_width, N, K);
		}
	}

	fclose(input);
	fclose(output);

	delete[] left_buffer;
	delete[] right_buffer;
	delete[] result_buffer;

	return 0;
}