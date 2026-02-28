#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "alloc.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// global private variables for all functions

static struct header *head = NULL;
static int heap_limit = 0;
static void *heap_start = NULL;
static enum algs algo = FIRST_FIT;

void *alloc(int size) {
  // if not enough space return NULL
  if (size <= 0) {
    return NULL;
  }

  // testing
  // total bytes needed to be allocated
  uint64_t total = (uint64_t)size + sizeof(struct header);
  if (total % 8 != 0) {
    // check if mutplie of 8
    // add padding to reach mutple
    total = total + (8 - (total % 8));
  }
  char *current_break = (char *)sbrk(0);

  // current bytes used
  // heap_start declared in allocation step at allocop
  int bytes_used = current_break - (char *)heap_start;

  // if allocation is above the limit, do nothing.
  // if (bytes_used + total > heap_limit) {
  //  return NULL;
  //}

  // request memory from free List
  struct header *curr = head;
  struct header *prev = NULL;
  struct header *chosen = NULL;
  struct header *chosen_prev = NULL;

  // looping through list to check free blocks

  while (curr != NULL) {
    if (curr->size >= total) {
      if (algo == FIRST_FIT) {
        chosen = curr;      // replace header address
        chosen_prev = prev; // old header points to new
        break;
      } else if (algo == BEST_FIT) {

        if (chosen == NULL || curr->size < chosen->size) {
          chosen = curr;
          chosen_prev = prev;
        }
      } else // WORST_FIT
      {
        if (chosen == NULL || curr->size > chosen->size) {
          chosen = curr;
          chosen_prev = prev;
        }
      }
    }
    prev = curr;
    curr = curr->next;
  }

  // grow the heap if empty
  if (chosen == NULL) {
    /// if (bytes_used + INCREMENT > heap_limit) {
    // return NULL;
    //}

    // request increment defined size with sbrk
    int requested_size = INCREMENT;

    // must be mutiple of INCREMENT.
    while (requested_size < total) {
      requested_size += INCREMENT;
    }

    if (bytes_used + requested_size > heap_limit) {
      return NULL;
    }

    struct header *new_block = (struct header *)sbrk(requested_size);

    // if invalid then return null
    if (new_block == (void *)-1) {
      return NULL;
    }

    // allocate in list
    new_block->size = requested_size;
    new_block->next = head;
    head = new_block;

    chosen = new_block;
    chosen_prev = NULL;
  }

  // remove from free list
  if (chosen_prev == NULL) {
    head = chosen->next;
  } else {
    chosen_prev->next = chosen->next;
  }

  // if large enough split into blocks
  //
  int remainder = chosen->size - total;
  int header_size = sizeof(struct header);

  if (remainder > header_size) {
    struct header *left_over = (struct header *)((char *)chosen + total);

    left_over->size = remainder;
    left_over->next = head;
    head = left_over;
    chosen->size = total;
  }

  chosen->next = NULL;

  return (void *)(chosen + 1);
}

void allocopt(enum algs alg, int limit) {
  // choose program (FIST_FIT,BEST_FIT,WORST_FIT)
  algo = alg;

  // max heap size defined
  heap_limit = limit;

  // check if heap has been used before
  if (heap_start == NULL) {
    // move address and discard all prev address
    heap_start = sbrk(0);
  } else {
    brk(heap_start);
  }

  // save current break point, without movin it

  // reset free block list.
  head = NULL;
}

void dealloc(void *ptr) {

  if (ptr == NULL) {
    return;
  }

  // compiler method of pointer arthmetic, so i dont need to calculate byte
  // increments.
  struct header *block = (struct header *)ptr - 1;

  struct header *curr = head;
  struct header *prev = NULL;

  // coalesce implemetation to merge blocks.
  while (curr != NULL) {
    // memory boundaries
    char *current_pos = (char *)curr;
    char *current_end = current_pos + curr->size;

    char *block_pos = (char *)block;
    char *block_end = block_pos + block->size;

    // check if curr is right before freed block
    if (current_end == block_pos) {
      // merge and remove curr from list
      if (prev != NULL) {
        prev->next = curr->next;
      } else {
        head = curr->next;
      }

      // merge and expand block of current
      curr->size += block->size;
      block = curr;

      // check for other main merges after expansion
      curr = head;
      prev = NULL;
      continue;
    }
    // case 2: if current is after freed block..
    else if (block_end == current_pos) {
      // remove curr from list
      if (prev != NULL) {
        prev->next = curr->next;
      } else {
        head = curr->next;
      }

      // merge and expand current
      block->size += curr->size;
      curr = block;

      // resume search after expansion
      curr = head;
      prev = NULL;
      continue;
    }
    prev = curr;
    curr = curr->next;
  }
  // from following the last, complete the coalsced block.
  block->next = head; // last goes to head, most recent.
  head = block;
}

struct allocinfo allocinfo(void) {
  struct allocinfo info;
  info.free_size = 0;
  info.free_chunks = 0;
  info.largest_free_chunk_size = 0;
  info.smallest_free_chunk_size = 0;

  struct header *curr = head;

  while (curr != NULL) {
    int available_space = curr->size - sizeof(struct header);

    info.free_size += available_space;
    info.free_chunks++;

    // if only one space avaiakble..
    if (info.free_chunks == 1) {
      info.largest_free_chunk_size = available_space;
      info.smallest_free_chunk_size = available_space;
    } else {

      // assign space to larger chunk.
      if (available_space > info.largest_free_chunk_size) {
        info.largest_free_chunk_size = available_space;
      }
      // assign space to smaller chink/
      if (available_space < info.smallest_free_chunk_size) {
        info.smallest_free_chunk_size = available_space;
      }
    }

    curr = curr->next;
  }

  return info; // 0 for now}
}
