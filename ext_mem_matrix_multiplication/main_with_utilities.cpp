#include <fstream>
#include <cstdint>
#include <algorithm>

// #include <cstdio>
#include <ctime>

const unsigned int BLOCK = 400;
const unsigned int SIZE_OFFSET = 2 * sizeof(uint32_t);

void print_matrix_from_file(const char* path) {
	printf("Print matrix\n");
	std::ifstream in(path, std::ifstream::in | std::ifstream::binary);

	uint32_t rows = 0;
	uint32_t cols = 0;

	in.read((char*)&rows, sizeof(rows));
	in.read((char*)&cols, sizeof(cols));

	printf("%d %d\n", rows, cols);

	uint8_t el = 0;
	char* elPtr = (char*)&el;
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			in.read(elPtr, sizeof(el));
			printf("%3d ", (size_t)el);
		}
		printf("\n");
	}
	printf("\n");

	in.close();
}

void print_block_info(size_t upper_row, size_t buffer_height, size_t left_col, size_t buffer_width) {
	printf_s("Processing block rows : %d - %d, cols : %d - %d\n",
		upper_row, upper_row + buffer_height,
		left_col, left_col + buffer_width);
}

void read_block(std::ifstream &in, size_t stream_offset, 
	uint8_t *buffer,
	size_t upper_row, size_t buffer_height, size_t left_col, size_t buffer_width,
	size_t row_count, size_t col_count) {

	size_t upper_left_offset_read = stream_offset + upper_row * col_count + left_col;
	if (buffer_width == col_count) {
		in.seekg(upper_left_offset_read, in.beg);
		in.read((char*)&buffer[0], buffer_width * buffer_height);
	}
	else {
		for (size_t i = 0; i < buffer_height; ++i){
			in.seekg(upper_left_offset_read + i * col_count, in.beg);
			in.read((char*)&buffer[buffer_width * i], buffer_width);
		}
	}
}

void write_block(std::ofstream &out, size_t stream_offset,
	uint8_t *buffer,
	size_t upper_row, size_t buffer_height, size_t left_col, size_t buffer_width,
	size_t row_count, size_t col_count) {

	size_t upper_left_offset_write = stream_offset + upper_row * col_count + left_col;
	out.seekp(upper_left_offset_write, out.beg);
	
	if (buffer_width == col_count) {
		out.seekp(upper_left_offset_write, out.beg);
		out.write((char*)&buffer[0], buffer_width * buffer_height);
	}
	else {
		for (size_t i = 0; i < buffer_height; ++i){
			out.seekp(upper_left_offset_write + i * col_count, out.beg);
			out.write((char*)&buffer[buffer_width * i], buffer_width);
		}
	}
	/*for (size_t i = 0; i < buffer_width; ++i) {
		if (!(buffer_height == row_count)) {
			out.seekp(upper_left_offset_write + i * row_count, out.beg);
		}
		for (size_t j = 0; j < buffer_height; ++j) {
			out.write((char*)&buffer[j * buffer_width + i], sizeof(uint8_t));
		}
	}*/
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

int test_main() {
	std::ifstream in("input_2500.bin", std::ifstream::in | std::ifstream::binary);
	std::ifstream out("output_2500.bin", std::ifstream::in | std::ifstream::binary);

	uint32_t n1 = 0;
	uint32_t n2 = 0;

	in.read((char*)&n1, sizeof(n1));
	in.read((char*)&n1, sizeof(n1));

	out.read((char*)&n2, sizeof(n2));
	out.read((char*)&n2, sizeof(n2));

	if (n1 != n2) {
		printf("n1 not equal to n2!\n");
		return 0;
	}

	uint8_t *buffer = new uint8_t[n1 * n1];
	uint8_t *result = new uint8_t[n1 * n1];
	in.read((char *)&buffer[0], n1 * n1);

	for (int i = 0; i < n1; ++i) {
		for (int j = 0; j < n1; ++j) {
			result[i * n1 + j] = 0;
			for (int k = 0; k < n1; ++k) {
				result[i * n1 + j] += buffer[i * n1 + k] * buffer[k * n1 + j];
			}
		}
	}

	uint8_t el = 0;
	for (int i = 0; i < n1 * n1; ++i) {
		out.read((char*)&el, sizeof(el));
		if (result[i] != el) {
			printf("results are not equal!\n");
			return 0;
		}
	}

	return 0;
}

int main() {
	// long start = clock();
	
	const char* input_filename = "input_2500.bin";
	const char* output_filename = "output_2500.bin";
	
	bool debug = false;

	std::ifstream in(input_filename, std::ifstream::in | std::ifstream::binary);
	std::ofstream out(output_filename, std::ofstream::out | std::ofstream::binary);

	// print_matrix_from_file(input_filename);

	uint32_t N = 0;
	
	in.read((char*)&N, sizeof(N));

	out.write((char*)&N, sizeof(N));
	out.write((char*)&N, sizeof(N));

	uint8_t *left_buffer = new uint8_t[BLOCK * BLOCK];
	uint8_t *right_buffer = new uint8_t[BLOCK * BLOCK];
	uint8_t *result_buffer = new uint8_t[BLOCK * BLOCK];

	size_t left_matrix_offset = SIZE_OFFSET;
	size_t right_matrix_offset = SIZE_OFFSET + N * N + SIZE_OFFSET;
	size_t result_matrix_offset = SIZE_OFFSET;

	for (size_t upper_row_result = 0; upper_row_result < N; upper_row_result += BLOCK) {
		size_t result_height = std::min(BLOCK, N - upper_row_result);
		for (size_t left_col_result = 0; left_col_result < N; left_col_result += BLOCK) {
			size_t result_width = std::min(BLOCK, N - left_col_result);
			
			size_t left_matrix_height = result_height;
			size_t right_matrix_width = result_width;

			for (size_t k = 0; k < N; k += BLOCK) {
				size_t left_matrix_width = std::min(BLOCK, N - k);
				size_t right_matrix_height = left_matrix_width;

				// read left matrix
				read_block(in, left_matrix_offset, left_buffer, upper_row_result, left_matrix_height, k, left_matrix_width, N, N);

				// read right matrix
				read_block(in, right_matrix_offset, right_buffer, k, right_matrix_height, left_col_result, right_matrix_width, N, N);
				
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

				// write result matrix
				write_block(out, result_matrix_offset, result_buffer, upper_row_result, result_height, left_col_result, result_width, N, N);
				
				/*printf("----------------\n");
				print_block_info(upper_row_result, left_matrix_height, k, left_matrix_width);
				printf("X\n");
				print_block_info(k, right_matrix_height, left_col_result, right_matrix_width);
				printf("=\n");
				print_block_info(upper_row_result, result_height, left_col_result, result_width);
				printf("----------------\n");*/
			}
		}
	}

	in.close();
	out.close();

	delete[] left_buffer;
	delete[] right_buffer;
	delete[] result_buffer;

	// printf("%.5f\n", (clock() - start) / (double)CLOCKS_PER_SEC);
	// print_matrix_from_file(output_filename);
	return 0;
}