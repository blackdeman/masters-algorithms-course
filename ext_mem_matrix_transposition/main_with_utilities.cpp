#include <fstream>
#include <cstdint>
#include <array>

// #include <cstdio>
// #include <ctime>

const unsigned int BLOCK = 300;
const unsigned int STREAM_OFFSET = 2 * sizeof(uint32_t);

/*
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
*/

class Buffer {
public:
	Buffer(uint32_t row_count, uint32_t col_count): 
		row_count(row_count),
		col_count(col_count) {
		buffer = new uint8_t[BLOCK * BLOCK];
	}
	~Buffer() {
		delete[] buffer;
	}

	void process_matrix_part(std::ifstream &in, std::ofstream &out,
		size_t upper_row, size_t buffer_height,
		size_t left_col, size_t buffer_width) {
		/*printf_s("Processing block col : %d - %d, rows : %d - %d\n",
		left_col, left_col + buffer_width,
		upper_row, upper_row + buffer_height);*/

		// Reading part from input
		size_t upper_left_offset_read = STREAM_OFFSET + upper_row * col_count + left_col;
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

		// Writing part to output
		size_t upper_left_offset_write = STREAM_OFFSET + left_col * row_count + upper_row;
		out.seekp(upper_left_offset_write, out.beg);
		for (size_t i = 0; i < buffer_width; ++i) {
			if (!(buffer_height == row_count)) {
				out.seekp(upper_left_offset_write + i * row_count, out.beg);
			}
			for (size_t j = 0; j < buffer_height; ++j) {
				out.write((char*)&buffer[j * buffer_width + i], sizeof(uint8_t));
			}
		}
	}
	/*
	void print_buffer() {
		for (size_t i = 0; i < BLOCK; ++i) {
			for (size_t j = 0; j < BLOCK; ++j) {
				printf("%3d ", buffer[i * BLOCK + j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	*/
private:
	size_t row_count;
	size_t col_count;

	//std::array<uint8_t, BLOCK * BLOCK> buffer;
	uint8_t *buffer;
};

int main() {
	// long start = clock();

	const char* input_filename = "input.bin";
	const char* output_filename = "output.bin";
	
	bool debug = false;

	std::ifstream in(input_filename, std::ifstream::in | std::ifstream::binary);
	std::ofstream out(output_filename, std::ofstream::out | std::ofstream::binary);

	// print_matrix_from_file(input_filename);

	uint32_t rows = 0;
	uint32_t cols = 0;
	
	in.read((char*)&rows, sizeof(rows));
	in.read((char*)&cols, sizeof(cols));

	out.write((char*)&cols, sizeof(cols));
	out.write((char*)&rows, sizeof(rows));
	
	Buffer buffer(rows, cols);

	// Calculate suitable blocks to process
	size_t row_step = 0;
	size_t col_step = 0;
	if (rows < BLOCK) {
		if (cols < BLOCK) {
			row_step = rows;
			col_step = cols;
		}
		else {
			row_step = rows;
			col_step = BLOCK * BLOCK / rows;
		}
	}
	else {
		if (cols < BLOCK) {
			row_step = BLOCK * BLOCK / cols;
			col_step = BLOCK;
		}
		else {
			row_step = BLOCK;
			col_step = BLOCK;
		}
	}


	for (size_t upper_row = 0; upper_row < rows; upper_row += row_step) {
		size_t cur_buffer_height = std::min(row_step, rows - upper_row);
		for (size_t left_col = 0; left_col < cols; left_col += col_step) {
			size_t cur_buffer_width = std::min(col_step, cols - left_col);
			buffer.process_matrix_part(in, out, upper_row, cur_buffer_height, left_col, cur_buffer_width);
			// buffer.print_buffer();
		}
	}
	
	in.close();
	out.close();

	// printf("%.5f", (clock() - start) / (double)CLOCKS_PER_SEC);
	// print_matrix_from_file(output_filename);
	return 0;
}