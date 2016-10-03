#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[] ) {

    int seed_count = 1000;

    srand(time(NULL));

    FILE * seeds = fopen("seeds_1000.txt", "w+");

    fprintf(seeds, "%d\n", seed_count);

    for (int i = 0; i < seed_count; ++i) {
        fprintf(seeds, "%d %d\n", rand(), rand());
    }

    return 0;
}