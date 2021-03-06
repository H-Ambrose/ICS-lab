
//zhu 1800012988@pku.edu.cn
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "cachelab.h"

#define IMAX 0x3f3f3f3f
unsigned hit = 0, miss = 0, eviction = 0, verbose = 0;
struct Cache {
	unsigned s, E, b;
	unsigned ***ca;//[i][j][k]:i-line;j-block;k-valid,tag,b
} cache;
void PrintUsageInfo();
void InitialCache(unsigned s, unsigned E, unsigned b) {
	cache.s = s;
	cache.E = E;
	cache.b = b;
	cache.ca = (unsigned ***)malloc(cache.s * sizeof(unsigned **));
	for (int i = 0;i < cache.s;++i)
		cache.ca[i] = (unsigned **)malloc(cache.E * sizeof(unsigned *));
	for (int i = 0;i < cache.s;++i)
		for (int j = 0;j < cache.E;++j)
			cache.ca[i][j] = (unsigned *)malloc(3 * sizeof(unsigned));
	for (int i = 0;i < cache.s;++i)
		for (int j = 0;j < cache.E;++j) {
			cache.ca[i][j][0] = 0;
			cache.ca[i][j][1] = 0;
			cache.ca[i][j][2] = IMAX;
		}
}
void Myfree() {
	for (int i = 0;i < cache.s;++i)
		for (int j = 0;j < cache.E;++j)
			free(cache.ca[i][j]);
	for (int i = 0;i < cache.s;++i)
		free(cache.ca[i]);
	free(cache.ca);
}
int findEorMiss(unsigned Sinca, unsigned tag) {
	for (int i = 0;i < cache.E;++i)
		if (cache.ca[Sinca][i][0] == 1 && cache.ca[Sinca][i][1] == tag)
			return i;
	return -1;
}
unsigned findNewE(unsigned Sinca) {
	unsigned LRU = IMAX;
	unsigned newE = 0;
	for (int i = 0;i < cache.E;++i) {
		if (cache.ca[Sinca][i][0] == 0)return i;
		unsigned tmp = cache.ca[Sinca][i][2];
		if (tmp < LRU) {
			LRU = tmp;
			newE = i;
		}
	}
	return newE;
}
void replace(unsigned Sinca, unsigned newE, unsigned tag) {
	if (cache.ca[Sinca][newE][0] == 1 && cache.ca[Sinca][newE][1] != tag) {
		++eviction;
		if (verbose)printf("eviction ");
	}
	cache.ca[Sinca][newE][0] = 1;
	cache.ca[Sinca][newE][1] = tag;
	cache.ca[Sinca][newE][2] = IMAX;
	for (int i = 0;i < cache.E;++i)
		if (i != newE)
			--cache.ca[Sinca][i][2];
}
void work(unsigned Sinca, unsigned tag) {
	int EorMiss = findEorMiss(Sinca, tag);
	if (EorMiss == -1) {
		++miss;
		if (verbose)printf("miss ");
		EorMiss = findNewE(Sinca);//return an unsigned
	}
	else {
		++hit;
		if (verbose)printf("hit ");
	}
	replace(Sinca, EorMiss, tag);
}
int main(int argc, char *const argv[]) {
	char op;
	FILE *path;
	int s = -1, E = -1, b = -1;
	while ((op = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (op) {
		case 'h': PrintUsageInfo();break; // print usage
		case 'v': verbose = 1;break;
		case 's': s = atoi(optarg);break;
		case 'E': E = atoi(optarg);break;
		case 'b': b = atoi(optarg);break;
		case 't': path = fopen(optarg, "r");break;
		default: printf("Invalid Input\n");break;
		}
	}
	if (s < 0 || E < 0 || b < 0) {
		printf("Invalid Cache\n");
		exit(1);
	}
	InitialCache(1 << (unsigned)s, E, 1 << (unsigned)b);
	unsigned addr, size;
	char opt[2];
	while (fscanf(path, "%s%x,%d", opt, &addr, &size) != EOF) {
		if (opt[0] == 'I')continue;
		if (verbose)printf("%c %x,%d ", opt[0], addr, size);
		unsigned tag = addr >> (s + b);
		unsigned Sinca = (addr&((1u << (s + b)) - 1)) >> b;
		if (opt[0] == 'L' || opt[0] == 'S')
			work(Sinca, tag);
		else {
			work(Sinca, tag);
			work(Sinca, tag);
		}
		if (verbose)printf("\n");
	}
	printSummary(hit, miss, eviction);
	Myfree();
	return 0;
}
void PrintUsageInfo() {
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("-h         Print this help message.\n");
	printf("-v         Optional verbose flag.\n");
	printf("-s <num>   Number of set index bits.\n");
	printf("-E <num>   Number of lines per set.\n");
	printf("-b <num>   Number of block offset bits.\n");
	printf("-t <file>  Trace file.\n\n\n");
	printf("Examples:\n");
	printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}