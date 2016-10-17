#include <fstream>
#include <cstdint>
#include <algorithm>

const unsigned int BLOCK = 400;
const unsigned int SIZE_OFFSET = 2 * sizeof(uint32_t);

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
}

int main() {	
	const char* input_filename = "input.bin";
	const char* output_filename = "output.bin";
	
	std::ifstream in(input_filename, std::ifstream::in | std::ifstream::binary);
	std::ofstream out(output_filename, std::ofstream::out | std::ofstream::binary);

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

				// read left matrix part
				read_block(in, left_matrix_offset, left_buffer, upper_row_result, left_matrix_height, k, left_matrix_width, N, N);

				// read right matrix part
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

			}
			// write result matrix part
			write_block(out, result_matrix_offset, result_buffer, upper_row_result, result_height, left_col_result, result_width, N, N);
		}
	}

	in.close();
	out.close();

	delete[] left_buffer;
	delete[] right_buffer;
	delete[] result_buffer;

	return 0;
}