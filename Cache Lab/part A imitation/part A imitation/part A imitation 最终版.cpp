
//zhu 1800012988@pku.edu.cn
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "cachelab.h"
#define ulong unsigned long int
#define IMAX 0x3f3f3f3f
ulong hit = 0, miss = 0, eviction = 0, verbose = 0;
struct Cache {
	ulong s, E, b;
	ulong ***ca;//[i][j][k]:i-line;j-block;k-valid,tag,b
} cache;
void PrintUsageInfo();
void InitialCache(ulong s, ulong E, ulong b) {
	cache.s = s;
	cache.E = E;
	cache.b = b;
	cache.ca = (ulong ***)malloc(cache.s * sizeof(ulong **));
	for (int i = 0;i < cache.s;++i)
		cache.ca[i] = (ulong **)malloc(cache.E * sizeof(ulong *));
	for (int i = 0;i < cache.s;++i)
		for (int j = 0;j < cache.E;++j)
			cache.ca[i][j] = (ulong *)malloc(3 * sizeof(ulong));
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
int findEorMiss(ulong Sinca, ulong tag) {
	for (int i = 0;i < cache.E;++i)
		if (cache.ca[Sinca][i][0] == 1 && cache.ca[Sinca][i][1] == tag)
			return i;
	return -1;
}
ulong findNewE(ulong Sinca) {
	ulong LRU = IMAX;
	ulong newE = 0;
	for (int i = 0;i < cache.E;++i) {
		if (cache.ca[Sinca][i][0] == 0)return i;
		ulong tmp = cache.ca[Sinca][i][2];
		if (tmp < LRU) {
			LRU = tmp;
			newE = i;
		}
	}
	return newE;
}
void replace(ulong Sinca, ulong newE, ulong tag) {
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
void work(ulong Sinca, ulong tag) {
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
	InitialCache(1 << (ulong)s, E, 1 << (ulong)b);
	ulong addr, size;
	char opt[2];
	while (fscanf(path, "%s%lx,%lu", opt, &addr, &size) != EOF) {
		if (opt[0] == 'I')continue;
		if (verbose)printf("%c %lx,%lu ", opt[0], addr, size);
		ulong tag = addr >> (s + b);
		ulong Sinca = (addr&((1u << (s + b)) - 1)) >> b;
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