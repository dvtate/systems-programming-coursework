/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// access bitmap on size_t header
#define GET(p)       (*(size_t *)(p))
#define GET_SIZE(p)  (GET(p) & ~1L)
#define GET_ALLOC(p) (GET(p) & 0x1)

// ll to track free spaces
struct free_blk_head {
	size_t size;
	struct free_blk_head* next;
    struct free_blk_head* prev;
};



struct free_blk_head* free_list_head;
void* first_block_addr;

#define MIN_BLOCK_SIZE (ALIGN(sizeof(struct free_blk_head)) + SIZE_T_SIZE)

// look for free space
void* find_fit(size_t size) {
	struct free_blk_head* block = free_list_head->next;

	while (block != free_list_head) {
		if (block->size >= size)
			return block;
		block = block->next;
	}
	return NULL;
}

// unify adjacent free spaces
void coalesce(struct free_blk_head* header) {
	size_t* footer = (size_t*) ((char*) header + header->size - SIZE_T_SIZE);

	int next_alloc = 1, prev_alloc = 1;
	size_t* prev_footer = (size_t*) ((char*) header - SIZE_T_SIZE);
	struct free_blk_head* prev_header = (struct free_blk_head*) ((char*) header - GET_SIZE(prev_footer));
	struct free_blk_head* next_header = (struct free_blk_head*) ((char*) header + header->size);

	if ((void*) prev_footer > first_block_addr)
		prev_alloc = GET_ALLOC(prev_footer);

	if ((void*) next_header < mem_heap_hi())
		next_alloc = GET_ALLOC(next_header);

	if (prev_alloc && next_alloc) {
		free_list_head->next->prev = header;
		free_list_head->next = header;

		*footer = *footer & ~1L;

	} else if (!prev_alloc && next_alloc) {

		size_t newsize = prev_header->size + header->size;
		prev_header->size = newsize;

		*footer = newsize;

	} else if (prev_alloc && !next_alloc) {

		size_t newsize = header->size + next_header->size;
		header->size = newsize;

		next_header->prev->next = header;
		next_header->next->prev = header;
		header->next = next_header->next;
		header->prev = next_header->prev;

		footer = (size_t*) ((char*) footer + next_header->size);
		*footer = newsize;

	} else {

		size_t newsize = prev_header->size + header->size + next_header->size;
		prev_header->size = newsize;

		// remove node
		next_header->next->prev = next_header->prev;
		next_header->prev->next = next_header->next;

		footer = (size_t*) ((char*) footer + next_header->size);
		*footer = newsize;

	}
}


/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    size_t size = ALIGN(sizeof(struct free_blk_head));
    free_list_head = mem_sbrk(size);
    if (free_list_head == (void*) -1)
        return -1;

    // init head
    free_list_head->size = 0;
    free_list_head->next = free_list_head;
    free_list_head->prev = free_list_head;

    // first block after free space
    first_block_addr = (char*) free_list_head + size;

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

	size_t new_size = ALIGN(size + SIZE_T_SIZE * 2);
	new_size = new_size > MIN_BLOCK_SIZE ? new_size : MIN_BLOCK_SIZE;

	size_t* free_block = find_fit(new_size);

    if (!free_block) {
        // dynamic padding based on size
        // new_size += 16;
        // new_size += new_size / 8;
        // new_size -= new_size % 8;
        new_size = ALIGN(new_size + new_size / 8);
        // no free space found, need to grow heap
        if ((free_block = mem_sbrk(new_size)) == (void*) -1)
            return NULL;
    } else {

        struct free_blk_head* h = (struct free_blk_head*) free_block;

        if (h->size >= new_size + MIN_BLOCK_SIZE) {
            // split block

            struct free_blk_head* new_head = (struct free_blk_head*) ((char*) h + new_size);
            new_head->size = h->size - new_size;
            new_head->next = h->next;
            new_head->prev = h->prev;

            size_t* footer = (size_t*) ((char*) h + h->size - SIZE_T_SIZE);
            *footer = new_head->size;

            // update ll
            h->next->prev = new_head;
            h->prev->next = new_head;
        } else {

            new_size = h->size;
            h->next->prev = h->prev;
            h->prev->next = h->next;
        }
    }

	*free_block = new_size | 1;
	size_t* footer = (size_t*) ((char*) free_block + new_size - SIZE_T_SIZE);
	*footer = new_size | 1;

	return (char *) free_block + SIZE_T_SIZE;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    struct free_blk_head* header = (struct free_blk_head*) ((char*) ptr - SIZE_T_SIZE);
    header->size = GET_SIZE(header);
    header->next = free_list_head->next;
    header->prev = free_list_head;

    coalesce(header);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return NULL;

    void* ret = mm_malloc(size);

    size_t len = *(size_t*)((char*)ptr - SIZE_T_SIZE);
	if (size < len)
		len = size;
    memcpy(ret, ptr, len);
    mm_free(ptr);
    return ret;
}
