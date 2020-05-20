/*
 * mm.c
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT - 1)) & ~0x7)
/* Basic constants and macros */
#define WSIZE 4            /* Word and header/footer size (bytes) */
#define DSIZE 8            /* Double word size (bytes) */
#define CHUNKSIZE (1 << 8) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read and write a double word at address p */
#define GETDS(p) ((char *)(*(unsigned long *)(p)))
#define PUTDS(p, val) (*(unsigned long *)(p) = ((unsigned long)(val)))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
/* */
#define PRED(bp) ((char *)(bp))
#define SUCC(bp) ((char *)(bp) + DSIZE)

/* Global variables */
static char *heap_listp = 0; /* Pointer to first block */
static char *freelist = 0;

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void *coalesce_for_place(void *bp);
static void *coalesce_for_place2(void *bp);
static void add_node(void *bp, size_t asize);
static void delete_node(void *bp);
static int getindex(size_t asize);
static void *realloc_usingmalloc(void *ptr, size_t size);
/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
    /* Create the initial empty heap */
    if((heap_listp = mem_sbrk(20 * WSIZE)) == (void *)-1) return -1;
    PUT(heap_listp, 0); /* Alignment padding */

    PUTDS(heap_listp + (WSIZE), 0); /* 一个pointer的大小是DSIZE */
    PUTDS(heap_listp + (3 * WSIZE), 0);
    PUTDS(heap_listp + (5 * WSIZE), 0);
    PUTDS(heap_listp + (7 * WSIZE), 0);
    PUTDS(heap_listp + (9 * WSIZE), 0);
    PUTDS(heap_listp + (11 * WSIZE), 0);
    PUTDS(heap_listp + (13 * WSIZE), 0);
    PUTDS(heap_listp + (15 * WSIZE), 0);

    /* heap的head和tail正常设置 */
    PUT(heap_listp + (17 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (18 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (19 * WSIZE), PACK(0, 1));

    freelist = heap_listp + WSIZE;
    heap_listp += (9 * DSIZE);
    // printf("init: %llx\n",(unsigned long long)heap_listp);
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;
    /*
    #ifdef DEBUG
            mm_checkheap(112);
    #endif
    */
    return 0;
}

/*
 * malloc
 */
void *malloc(size_t size) {
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    if(heap_listp == 0) {
        mm_init();
    }
    /* Ignore spurious requests */
    if(size == 0) return NULL;

    /* 跟textbook相比多了pred和succ的大小 */
    /* 笔记：size<=2*DSIZE而不是size<=DSIZE */
    /* 我觉得我永远都不能理解为什么在else里面：
     * ALIGN(size + DSIZE)就会mem_sbrk failed
     * 只有((size + (DSIZE) + (DSIZE - 1)) / DSIZE)才是对的
     */
    if(size <= 2 * DSIZE)
        asize = 3 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if((bp = find_fit(asize)) != NULL) {
        bp = place(bp, asize);
        /*
        #ifdef DEBUG
                mm_checkheap(147);
        #endif
        */
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
    bp = place(bp, asize);
    /*
    #ifdef DEBUG
            mm_checkheap(158);
    #endif
    */
    return bp;
}

/*
 * free
 */
void free(void *bp) {
    if(bp == 0) return;

    size_t size = GET_SIZE(HDRP(bp));
    if(heap_listp == 0) {
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    /* for free list */
    PUT(PRED(bp), 0);
    PUT(SUCC(bp), 0);
    coalesce(bp);
}

/*
 * realloc - you may want to look at mm-naive.c
 * 等着像隐式优化一样优化这里:优化之后貌似还降了1分???
 */
void *realloc(void *ptr, size_t size) {
    size_t oldsize, asize;
    void *newptr = ptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return malloc(size);
    }

    if(size <= 2 * DSIZE)
        asize = 3 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    oldsize = GET_SIZE(HDRP(ptr));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    size_t prev_size = GET_SIZE(HDRP(PREV_BLKP(ptr)));

    size_t sumsize;

    if(!next_alloc && !prev_alloc) { /* next和prev都是空的 */
        sumsize = next_size + prev_size + oldsize;
        if(sumsize >= asize) {
            delete_node(NEXT_BLKP(ptr));
            delete_node(PREV_BLKP(ptr));
            ptr = PREV_BLKP(ptr);
            if(sumsize - asize >= 3 * DSIZE) {
                PUT(HDRP(ptr), PACK(asize, 1));
                PUT(FTRP(ptr), PACK(asize, 1));
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                coalesce_for_place(NEXT_BLKP(ptr));
            }
            else {
                PUT(HDRP(ptr), PACK(sumsize, 1));
                PUT(FTRP(ptr), PACK(sumsize, 1));
            }
            return ptr;
        }
        else
            return realloc_usingmalloc(ptr, asize);
    }
    else if(!next_alloc && prev_alloc) { /* next是空的,prev非空 */
        sumsize = next_size + oldsize;
        if(sumsize >= asize) {
            delete_node(NEXT_BLKP(ptr));
            if(sumsize - asize >= 3 * DSIZE) {
                PUT(HDRP(ptr), PACK(asize, 1));
                PUT(FTRP(ptr), PACK(asize, 1));
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                coalesce_for_place(NEXT_BLKP(ptr));
            }
            else {
                PUT(HDRP(ptr), PACK(sumsize, 1));
                PUT(FTRP(ptr), PACK(sumsize, 1));
            }
            return ptr;
        }
        else
            return realloc_usingmalloc(ptr, asize);
    }
    else if(next_alloc && !prev_alloc) { /* prev是空的,next非空 */
        sumsize = prev_size + oldsize;
        if(sumsize >= asize) {
            delete_node(PREV_BLKP(ptr));
            ptr = PREV_BLKP(ptr);
            if(sumsize - asize >= 3 * DSIZE) {
                PUT(HDRP(ptr), PACK(asize, 1));
                PUT(FTRP(ptr), PACK(asize, 1));
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(sumsize - asize, 0));
                coalesce_for_place(NEXT_BLKP(ptr));
            }
            else {
                PUT(HDRP(ptr), PACK(sumsize, 1));
                PUT(FTRP(ptr), PACK(sumsize, 1));
            }
            return ptr;
        }
        else
            return realloc_usingmalloc(ptr, asize);
    }
    else if(next_alloc && prev_alloc) { /* next和prev都非空 */
        if(oldsize >= asize) {
            /* 不需delete node，因为ptr所在的块一定被占用了 */
            if(oldsize - asize >= 3 * DSIZE) {
                PUT(HDRP(ptr), PACK(asize, 1));
                PUT(FTRP(ptr), PACK(asize, 1));
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(oldsize - asize, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(oldsize - asize, 0));
                coalesce_for_place(NEXT_BLKP(ptr));
            }
            else {
                PUT(HDRP(ptr), PACK(oldsize, 1));
                PUT(FTRP(ptr), PACK(oldsize, 1));
            }
            return ptr;
        }
        else
            return realloc_usingmalloc(ptr, asize);
    }
    return newptr;
}

static void *realloc_usingmalloc(void *ptr, size_t size) {
    size_t oldsize = GET_SIZE(HDRP(ptr));
    void *newptr = malloc(size);
    if(!newptr) return 0;
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);
    free(ptr);
    return newptr;
}

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;
    /* Allocate an even number of words to maintain alignment */
    /* 8字节对齐 */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* for link list */
    PUT(PRED(bp), 0);
    PUT(SUCC(bp), 0);
    /* Coalesce if the previous block was free */
    /* bp指向空块的开头,coalesce的过程中bp不能改变 */
    return coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t asize = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) { /* Case 1 */
    }

    else if(prev_alloc && !next_alloc) { /* Case 2 */
        asize += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(asize, 0));
        PUT(FTRP(bp), PACK(asize, 0));
    }

    else if(!prev_alloc && next_alloc) { /* Case 3 */
        asize += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_node(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(asize, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(asize, 0));
        bp = PREV_BLKP(bp);
    }

    else { /* Case 4 */
        asize += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        delete_node(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 0));
        bp = PREV_BLKP(bp);
    }
    /* 初版在这里重新设置bp的pred和succ指针 */
    add_node(bp, asize);
    /*
    #ifdef DEBUG
            mm_checkheap(285);
    #endif
    */
    return bp;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void *place(void *bp, size_t asize) {
    void *returnbp = bp;
    size_t csize = GET_SIZE(HDRP(bp));
    size_t remainder = csize - asize;
    size_t magic_number = 1024;
    delete_node(bp);
    if(remainder >= (3 * DSIZE)) {
        if(asize >= magic_number) {
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));
            bp = NEXT_BLKP(bp);
            PUT(HDRP(bp), PACK(remainder, 0));
            PUT(FTRP(bp), PACK(remainder, 0));
            coalesce_for_place(bp);
            /* the prev block is allocated */
        }
        else {
            PUT(HDRP(bp), PACK(remainder, 0));
            PUT(FTRP(bp), PACK(remainder, 0));
            returnbp = NEXT_BLKP(bp);
            PUT(HDRP(NEXT_BLKP(bp)), PACK(asize, 1));
            PUT(FTRP(NEXT_BLKP(bp)), PACK(asize, 1));
            coalesce_for_place2(bp);
            /* the next block is allocated */
        }
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    return returnbp;
}

/*
 * coalesce_for_place2
 */
static void *coalesce_for_place2(void *bp) {
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t asize = GET_SIZE(HDRP(bp));

    if(prev_alloc) { /* 前一个块也是占用的 */
    }
    else {
        asize += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_node(PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(asize, 0));
        PUT(FTRP(bp), PACK(asize, 0));
    }
    /* 初版在这里重新设置bp的pred和succ指针 */
    add_node(bp, asize);
    return bp;
}

/*
 * coalesce_for_place
 */
static void *coalesce_for_place(void *bp) {
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t asize = GET_SIZE(HDRP(bp));

    if(next_alloc) { /* 后一个块也是占用的 */
    }
    else {
        asize += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(asize, 0));
        PUT(FTRP(bp), PACK(asize, 0));
    }
    /* 初版在这里重新设置bp的pred和succ指针 */
    add_node(bp, asize);
    return bp;
}

void add_node(void *bp, size_t asize) {
    /* heap起始处标记的是链表的结尾 */
    void *lst = freelist + getindex(asize) * DSIZE;
    PUTDS(SUCC(bp), NULL);
    PUTDS(PRED(bp), NULL);
    if(GETDS(lst) == NULL) { /* empty free list */
        PUTDS(SUCC(bp), NULL);
        PUTDS(lst, bp);
    }
    else {
        void *p1 = GETDS(lst); /* p1 will never be NULL */
        void *p2 = NULL;
        while((p1 != NULL) && (asize > GET_SIZE(HDRP(p1)))) {
            p2 = p1;
            p1 = GETDS(SUCC(p1));
        }
        /* p1!=NULL,p2==NULL:
         * 说明循环没有执行，已知第一次p1!=NULL，说明一定是asize<=p1
         * 对应的块的size,该块应该插入到链表头,并且lst变为bp
         * p1==NULL,p2!=NULL:
         * 说明该块应该插入到链表末端，且已知这不是一条空链表
         * p1!=NULL,p2!=NULL:
         * 正常add node
         */
        if(p1 == NULL) { /* lst don't have a succeed */
            /* p2==NULL是不可能的 */
            PUTDS(PRED(bp), p2);
            PUTDS(SUCC(p2), bp);
        }
        else {
            if(p2 == NULL) { /* free list header */
                PUTDS(SUCC(bp), p1);
                PUTDS(PRED(p1), bp);
                PUTDS(lst, bp);
            }
            else {
                PUTDS(SUCC(bp), p1);
                PUTDS(PRED(p1), bp);
                PUTDS(PRED(bp), p2);
                PUTDS(SUCC(p2), bp);
            }
        }
        // PUTDS(SUCC(bp), GETDS(lst));
        // PUTDS(PRED(GETDS(lst)), bp);
        // PUTDS(lst, bp);
    }
    return;
}

void delete_node(void *bp) {
    void *pred = GETDS(PRED(bp));
    void *succ = GETDS(SUCC(bp));
    /* Reduce the number of intermediate variables */

    void *lst = freelist + getindex(GET_SIZE(HDRP(bp))) * DSIZE;
    if(pred == NULL && succ == NULL) { /* empty list */
        PUTDS(lst, NULL);
    }
    else if(pred != NULL && succ == NULL) { /* the end of list */
        PUTDS(SUCC(pred), NULL);
    }
    else if(pred == NULL && succ != NULL) { /* the start of list */
        PUTDS(PRED(succ), NULL);
        PUTDS(lst, succ);
    }
    else {
        PUTDS(SUCC(pred), succ);
        PUTDS(PRED(succ), pred);
    }
    PUTDS(PRED(bp), 0);
    PUTDS(SUCC(bp), 0);
    return;
}

int getindex(size_t asize) {
    if(asize == 16)
        return 0;
    else if(asize <= 64)
        return 1;
    else if(asize <= 128)
        return 2;
    else if(asize <= 256)
        return 3;
    else if(asize <= 512)
        return 4;
    else if(asize <= 1024)
        return 5;
    else if(asize <= 4096)
        return 6;
    else if(asize >= 5120)
        return 7;
    return 0;
}

/*
 * find_fit - Find a fit for a block with asize bytes
 * find first fit
 */
static void *find_fit(size_t asize) {
    void *bp;
    int i = getindex(asize);
    for(; i < 8; i++) {
        for(bp = GETDS(freelist + i * DSIZE); bp != NULL;
            bp = GETDS(SUCC(bp))) {
            if(asize <= GET_SIZE(HDRP(bp))) {
                return bp;
            }
        }
    }
    return NULL; /* No fit */
}

/*
 * calloc - Allocate the block and set it to zero.
 */
void *calloc(size_t nmemb, size_t size) {
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}

/*
 * mm_checkheap - Check the heap for correctness. Helpful hint: You
 *                can call this function using mm_checkheap(__LINE__);
 *                to identify the line number of the call site.
 */
void mm_checkheap(int lineno) {
    char *st = heap_listp;
    size_t size;
    size_t headsize, headalloc, footsize, footalloc;
    while((size = GET_SIZE(HDRP(st))) != 0) {
        headsize = GET_SIZE(HDRP(st));
        headalloc = GET_ALLOC(HDRP(st));
        footsize = GET_SIZE(FTRP(st));
        footalloc = GET_ALLOC(FTRP(st));
        if(headsize == 0) {
            printf("in line %d\n", lineno);
            printf("%p: Error, hollow header\n", st);
            exit(0);
        }
        if(footsize == 0) {
            printf("in line %d\n", lineno);
            printf("%p: Error, hollow footer\n", st);
            exit(0);
        }
        printf("%p: header: [%ld:%s] footer: [%ld:%s]\n",
               st,
               headsize,
               (headalloc ? "alloc" : "free"),
               footsize,
               (footalloc ? "alloc" : "free"));

        if((size_t)st % 8) {
            printf("in line %d\n", lineno);
            printf("Error: %p is not double word aligned\n", st);
            exit(0);
        }
        if(GET(HDRP(st)) != GET(FTRP(st))) {
            printf("in line %d\n", lineno);
            printf("Error: header does not match footer\n");
            exit(0);
        }

        if(!GET_ALLOC(HDRP(st)) && GETDS(PRED(st)) == 0 &&
           GETDS(SUCC(st)) == 0) {
            printf("in line %d: ", lineno);
            printf("%p :Warning, free block but not in linklist, ", st);
            printf("please recheck the code.\n");
            /* don't exit immediately */
        }
        st = NEXT_BLKP(st);
    }
}
