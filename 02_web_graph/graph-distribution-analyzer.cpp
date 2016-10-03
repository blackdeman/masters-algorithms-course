#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

int main(int argc, char* argv[])
{
	std::string dir_name (argv[1]);
	
	std::vector<int> degrees_global(100000, 0);
	int max_degree_global = 0;

	std::string output_average_name = dir_name + "\\output_average.distribution";
	FILE *output_average;
	fopen_s(&output_average, output_average_name.c_str(), "w+");

	int iterations = 2000;

	for (size_t i = 1; i <= iterations; ++i)
	{
		std::string input_file_name = dir_name + "\\output_" + std::to_string(i) + ".txt";
		std::string output_file_name = input_file_name + ".distribution";
		FILE *input, *output;
		fopen_s(&input, input_file_name.c_str(), "r");
		fopen_s(&output, output_file_name.c_str(), "w+");
		
		int v, e, v1, v2;
		fscanf_s(input, "%d %d", &v, &e);
		std::vector<int> degrees(v, 0);
		for (size_t j = 0; j < e; ++j)
		{
			fscanf_s(input, "%d %d", &v1, &v2);
			degrees[v1]++;
			degrees[v2]++;
		}
		int max_degree = *std::max_element(degrees.begin(), degrees.end());
		if (max_degree > max_degree_global)
			max_degree_global = max_degree;

		std::vector<int> degree_distrib(max_degree + 1, 0);
		for (size_t j = 0; j < degrees.size(); ++j)
		{
			degree_distrib[degrees[j]]++;
		}

		fprintf(output, "%d\n", degree_distrib.size());
		for (size_t j = 0; j < degree_distrib.size(); ++j)
		{
			fprintf(output, "%d %d\n", j, degree_distrib[j]);
			degrees_global[j] += degree_distrib[j];
		}

		fclose(output);
		fclose(input);
	}

	degrees_global.resize(max_degree_global + 1);
	fprintf(output_average, "%d\n", degrees_global.size());
	for (size_t i = 0; i < degrees_global.size(); ++i)
	{
		fprintf(output_average, "%d %.5f\n", i, degrees_global[i] / (double) iterations);
	}
	fclose(output_average);

	return 0;
}