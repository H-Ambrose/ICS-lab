/*
 * Simple, 32-bit and 64-bit clean allocator based on implicit free
 * lists, first-fit placement, and boundary tag coalescing, as described
 * in the CS:APP3e text. Blocks must be aligned to doubleword (8 byte)
 * boundaries. Minimum block size is 16 bytes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/*
 * If NEXT_FIT defined use next fit search, else use first-fit search
 */
#define NEXT_FITx

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/* Global variables */
static char *heap_listp = 0; /* Pointer to first block */
#ifdef NEXT_FIT
static char *rover; /* Next fit rover */
#endif

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);

/*
 * mm_init - Initialize the memory manager
 */
int mm_init(void) {
    /* Create the initial empty heap */
    /*((void *)-1)是把-1转换成指针0xFFFFFFFF，判断有无返回错误*/
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);

#ifdef NEXT_FIT
    rover = heap_listp;
#endif

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;
    return 0;
}

/*
 * malloc - Allocate a block with at least size bytes of payload
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

    /* Adjust block size to include overhead and alignment reqs. */
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
    place(bp, asize);
    return bp;
}

/*
 * free - Free a block
 */
void free(void *bp) {
    if(bp == 0) return;

    size_t size = GET_SIZE(HDRP(bp));
    if(heap_listp == 0) {
        mm_init();
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * realloc - Naive implementation of realloc
 */
void *realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/*
 * mm_checkheap - Check the heap for correctness. Helpful hint: You
 *                can call this function using mm_checkheap(__LINE__);
 *                to identify the line number of the call site.
 */
void mm_checkheap(int lineno) {
    lineno = lineno; /* keep gcc happy */
}

/*
 * The remaining routines are internal helper routines
 */

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc) { /* Case 1 */
        return bp;
    }

    else if(prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if(!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else { /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if((rover > (char *)bp) && (rover < NEXT_BLKP(bp))) rover = bp;
#endif
    return bp;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));

    if((csize - asize) >= (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize) {
#ifdef NEXT_FIT
    /* Next fit search */
    char *oldrover = rover;

    /* Search from the rover to the end of list */
    for(; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if(!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    /* search from start of list to old rover */
    for(rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if(!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;

    return NULL; /* no fit found */
#else
    /* First-fit search */
    void *bp;

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL; /* No fit */
#endif
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
                printf("check : case 1\n");
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
                printf("check : case 2\n");
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
                printf("check : case 3\n");
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
                printf("check : case 4\n");
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

    if(oldsize == asize)
        return ptr;

    else if(oldsize > asize) {
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
    /* ptr is used,不在free list中 */
    else if(oldsize < asize) {
        size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
        size_t prev_size = GET_SIZE(HDRP(PREV_BLKP(ptr))) + oldsize;
        if(!prev_alloc && prev_size >= asize) { /* 直接插入prev块，释放ptr */
            delete_node(PREV_BLKP(ptr));
            ptr = PREV_BLKP(ptr); /* old ptr in new ptr */
            memcpy(ptr, newptr, MIN(oldsize, size));
            if(prev_size - asize >= 3 * DSIZE) {
                PUT(HDRP(ptr), PACK(asize, 1));
                PUT(FTRP(ptr), PACK(asize, 1));
                PUT(HDRP(NEXT_BLKP(ptr)), PACK(prev_size - asize, 0));
                PUT(FTRP(NEXT_BLKP(ptr)), PACK(prev_size - asize, 0));
                add_node(NEXT_BLKP(ptr), prev_size - asize);
                free(newptr);
            }
            else {
                PUT(HDRP(ptr), PACK(prev_size, 1));
                PUT(FTRP(ptr), PACK(prev_size, 1));
            }
            return ptr;
        }

        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
        size_t sumsize = GET_SIZE(HDRP(NEXT_BLKP(ptr))) + oldsize;
        if(!next_alloc && sumsize >= asize) {
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
        else {
            newptr = malloc(size);
            if(!newptr) return 0;
            if(size < oldsize) oldsize = size;
            memcpy(newptr, ptr, oldsize);
            free(ptr);
        }
    }
    /*
    else {
        newptr = malloc(size);
        if(!newptr) return 0;
        if(size < oldsize) oldsize = size;
        memcpy(newptr, ptr, oldsize);
        free(ptr);
    }
    */
    /* 似乎不写if(oldsize < asize)的情况而是直接newptr效果是最好的??? */
    return newptr;
}
