#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cachelab.h"
unsigned hit = 0, miss = 0, eviction = 0, verbose = 0;
struct Cache {
    unsigned s, E, b;
    unsigned ***ca;
} cache;
int main(int argc,char *const argv[]) {

}