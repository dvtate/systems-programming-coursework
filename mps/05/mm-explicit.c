/* This source code accompanies the screencast at http://vimeo.com/22728196
 * Please note that the goal of the screencast was to quickly hack together a
 * rudimentary explicit-list based allocator, while simultaneously demonstrating
 * how to test, debug, and evaluate it using the provided driver.
 *
 * There are no provisions here for coalescing, splitting, or intelligent
 * realloc-ing. And you'll want them all!
 *
 * I do NOT recommend using it as the basis for your own allocator --- start
 * afresh and save yourself a lot of headache!
 *
 * - Michael <lee@iit.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

#define ALIGNMENT 8

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define BLK_HDR_SIZE ALIGN(sizeof(blockHdr))

typedef struct header blockHdr;

struct header {
  size_t size;
  blockHdr *next_p;
  blockHdr *prior_p;
};

void *find_fit(size_t size);
void print_heap();

int mm_init(void)
{
  blockHdr *p = mem_sbrk(BLK_HDR_SIZE);
  p->size = BLK_HDR_SIZE;
  p->next_p = p;
  p->prior_p = p;
  return 0;
}

void print_heap() {
  blockHdr *bp = mem_heap_lo();
  while (bp < (blockHdr *)mem_heap_hi()) {
    printf("%s block at %p, size %d\n",
           (bp->size&1)?"allocated":"free",
           bp,
           (int)(bp->size & ~1));
    bp = (blockHdr *)((char *)bp + (bp->size & ~1));
  }
}

void *mm_malloc(size_t size)
{
  int newsize = ALIGN(BLK_HDR_SIZE + size);
  blockHdr *bp = find_fit(newsize);
  if (bp == NULL) {
    bp = mem_sbrk(newsize);
    if ((long)bp == -1)
      return NULL;
    else
      bp->size = newsize | 1;
  } else {
    bp->size |= 1;
    bp->prior_p->next_p = bp->next_p;
    bp->next_p->prior_p = bp->prior_p;
  }
  return (char *)bp + BLK_HDR_SIZE;
}

void *find_fit(size_t size)
{
  blockHdr *p;
  for (p = ((blockHdr *)mem_heap_lo())->next_p;
       p != mem_heap_lo() && p->size < size;
       p = p->next_p);

  if (p != mem_heap_lo())
    return p;
  else
    return NULL;
}

void mm_free(void *ptr)
{
  blockHdr *bp = ptr-BLK_HDR_SIZE,
    *head = mem_heap_lo();
  bp->size &= ~1;
  bp->next_p  = head->next_p;
  bp->prior_p = head;
  head->next_p = bp;
  bp->next_p->prior_p = bp;
}

void *mm_realloc(void *ptr, size_t size)
{
  blockHdr *bp = ptr-BLK_HDR_SIZE;
  void *newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  int copySize = bp->size-BLK_HDR_SIZE;
  if (size < copySize)
    copySize = size;
  memcpy(newptr, ptr, copySize);
  mm_free(ptr);
  return newptr;
}
