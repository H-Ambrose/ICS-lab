#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cachelab.h"
#define INF 0x3f3f3f3f
#define FOR(i, a, b) for(int i = a; i < b; i++)
#define uint long unsigned int
uint Hits = 0, Misses = 0, Evictions = 0, verbose = 0;
struct _Cache {
    uint s, E, b;
    uint ***cache;
} Cache;
void Set_Cache(uint _s, uint _E, uint _b) {
    Cache.s = _s;
    Cache.E = _E, Cache.b = _b;
}
uint Get_Valid(uint _s, uint _E) {
    return Cache.cache[_s][_E][0];
}
uint Get_Tag(uint _s, uint _E) {
    return Cache.cache[_s][_E][1];
}
uint Get_Lru(uint _s, uint _E) {
    return Cache.cache[_s][_E][2];
}
void Get_Cache() {
    Cache.cache = (uint ***)malloc(Cache.s * sizeof(uint **));
    FOR(i, 0, Cache.s)
    Cache.cache[i] = (uint **)malloc(Cache.E * sizeof(uint *));
    FOR(i, 0, Cache.s)
    FOR(j, 0, Cache.E) Cache.cache[i][j] = (uint *)malloc(3 * sizeof(uint));
    FOR(i, 0, Cache.s) FOR(j, 0, Cache.E) {
        Cache.cache[i][j][0] = 0;
        Cache.cache[i][j][1] = 0;
        Cache.cache[i][j][2] = INF;
    }
}
void Free_Cache() {
    FOR(i, 0, Cache.s) FOR(j, 0, Cache.E) free(Cache.cache[i][j]);
    FOR(i, 0, Cache.s) free(Cache.cache[i]);
    free(Cache.cache);
}
int Is_Hit(uint _s, uint Tag) {
    FOR(i, 0, Cache.E)
    if(Cache.cache[_s][i][0] == 1 && Cache.cache[_s][i][1] == Tag) return i;
    return -1;
}
uint Get_Min_Lru(uint _s) {
    uint Lru = INF, Pos = 0;
    FOR(i, 0, Cache.E) {
        if(Get_Valid(_s, i) == 0) return i;
        if(Lru > Get_Lru(_s, i)) {
            Lru = Get_Lru(_s, i);
            Pos = i;
        }
    }
    return Pos;
}

void Replace(uint _s, uint _E, uint Tag) {
    if(Get_Valid(_s, _E) == 1 && Get_Tag(_s, _E) != Tag) {
        ++Evictions;
        if(verbose) printf("Eviction ");
    }
    Cache.cache[_s][_E][0] = 1;
    Cache.cache[_s][_E][1] = Tag;
    Cache.cache[_s][_E][2] = INF;
    FOR(i, 0, Cache.E) if(i != _E)-- Cache.cache[_s][i][2];
}
void Puts_Set(uint _s) {
    FOR(i, 0, Cache.E)
    printf("%lu %lx %lu\n",
           Cache.cache[_s][i][0],
           Cache.cache[_s][i][1],
           Cache.cache[_s][i][2]);
}
void Puts_Cache() {
    printf("S=%lu E=%lu B=%lu\n", Cache.s, Cache.E, Cache.b);
    printf("********************************\n\n");
    FOR(i, 0, Cache.s) {
        Puts_Set(i);
        printf("\n");
    }
    printf("********************************\n");
}
void Print_Help() {
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n\n");

    printf("Options:\n");
    printf("	-h         Print this help message.\n");
    printf("	-v         Optional verbose flag.\n");
    printf("	-s <num>   Number of set index bits.\n");
    printf("	-E <num>   Number of lines per set.\n");
    printf("	-b <num>   Number of block offset bits.\n");
    printf("	-t <file>  Trace file.\n\n");

    printf("Examples:\n");
    printf("linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
void Check_And_Solve(uint S, uint Tag) {
    int IS_HIT = Is_Hit(S, Tag);
    if(IS_HIT == -1) {
        ++Misses;
        if(verbose) printf("Misses ");
        Replace(S, Get_Min_Lru(S), Tag);
    }
    else {
        ++Hits;
        Replace(S, IS_HIT, Tag);
        if(verbose) printf("Hits ");
    }
}
int main(int argc, char *const argv[]) {
    _Bool wrong = 0;
    FILE *Path = NULL;
    char opt, cmd[2], Ch;
    int _s = -1, _E = -1, _b = -1;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 'h':
                Print_Help();
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                _s = atol(optarg);
                break;
            case 'E':
                _E = atol(optarg);
                break;
            case 'b':
                _b = atol(optarg);
                break;
            case 't':
                Path = fopen(optarg, "r");
                break;
            default:
                wrong = 1;
        }
    }
    if(_s <= 0 || _E <= 0 || _b <= 0 || Path == NULL || wrong) {
        Print_Help();
        return 0;
    }
    Set_Cache(1 << (uint)_s, _E, 1 << (uint)_b);
    Get_Cache();
    uint addr, number, Tag, S;
    while(fscanf(Path, "%s%lx%c%lu", cmd, &addr, &Ch, &number) != EOF) {
        if(cmd[0] == 'I') continue;
        Tag = addr >> (_s + _b);
        S = (addr & ((1lu << (_s + _b)) - 1)) >> _b;
        if(verbose) printf("%c %lx%c%lu: ", cmd[0], addr, Ch, number);
        if(cmd[0] == 'L' || cmd[0] == 'S')
            Check_And_Solve(S, Tag);
        else {
            Check_And_Solve(S, Tag);
            Check_And_Solve(S, Tag);
        }
        if(verbose) printf("S=%lu Tag=%lu\t\n", S, Tag);
    }
    printSummary(Hits, Misses, Evictions);
    Free_Cache();
    return 0;
}