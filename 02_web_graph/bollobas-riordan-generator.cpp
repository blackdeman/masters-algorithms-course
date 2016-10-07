#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <list>
#include <algorithm>
#include <random>

void print_vector(std::vector<std::list<int>> v)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		printf("%d : ", i);
		for (std::list<int>::iterator it_list = v[i].begin(); it_list != v[i].end(); ++it_list)
		{
			printf(" %d", *it_list);
		}
		printf("\n");
	}
}

void print_edges(std::vector<std::list<int>> v)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		for (std::list<int>::iterator it_list = v[i].begin(); it_list != v[i].end(); ++it_list)
		if (*it_list >= i)
		{
			printf("%d %d\n", i, *it_list);
		}
	}
}

int main(int argc, char *argv[])
{
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);

	bool debug_log = false;
	bool print_time = false;

	int num_vertex = N;
	int num_edges = N * M;

	printf("%d\n%d\n", N, num_edges);

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(0.0, 1.0);

	std::vector<int> degrees(num_edges, 0);
	std::vector<std::list<int>> connections_ext(num_edges, std::list<int>());
	std::vector<std::list<int>> connections(num_vertex, std::list<int>());

	long t_start = clock();

	degrees[0] = 2;
	connections_ext[0].push_back(0);

	for (int i = 1; i < num_edges; ++i)
	{
		double number = distribution(generator);
		degrees[i]++;
		if (debug_log)
		{
			printf("Random = %.5f\n", number);
			printf("Degrees : \n");
			for (int k = 0; k < i + 1; ++k)
			{
				printf("%4d", degrees[k]);
			}
			printf("\n");
		}
		double cum_degree_sum = 0;
		for (int j = 0; j < i + 1; ++j)
		{
			double from = cum_degree_sum / (double)(2 * (i + 1) - 1);
			double to = (cum_degree_sum + degrees[j]) / (double)(2 * (i + 1) - 1);
			cum_degree_sum += degrees[j];
			if (debug_log)
				printf("%.2f - %.2f ", from, to);
			if (number >= from && number <= to)
			{
				if (debug_log)
					printf("!!!");
				connections_ext[i].push_back(j);
				if (i != j)
				{
					connections_ext[j].push_back(i);
				}
				degrees[j]++;
				//break;
			}
			if (debug_log)
				printf(" ; ");
		}
		if (debug_log)
			printf("\n");
	}

	long t_after_first = clock();
	if (print_time)
		printf("First part gen time : %f\n", (t_after_first - t_start) / (double)CLOCKS_PER_SEC);

	if (debug_log)
		print_vector(connections_ext);

	for (size_t i = 0; i < connections_ext.size(); ++i)
	{
		int new_from_vertex = i / M;
		for (std::list<int>::iterator it_list = connections_ext[i].begin(); it_list != connections_ext[i].end(); ++it_list)
		{
			int new_to_vertex = *it_list / M;
			if (*it_list >= i)
			{
				connections[new_from_vertex].push_back(new_to_vertex);
			}
		}
	}
	
	long t_after_second = clock();
	if (print_time)
		printf("Second part gen time : %f\n", (t_after_second - t_after_first) / (double)CLOCKS_PER_SEC);

	if (print_time)
		printf("Total time : %f\n", (t_after_second - t_start) / (double)CLOCKS_PER_SEC);

	if (debug_log)
		print_vector(connections);

	print_edges(connections);

	return 0;
}