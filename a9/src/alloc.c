#include "alloc.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// Explicit declarations needed on platforms where <unistd.h> omits them
extern void *sbrk(intptr_t increment);
extern int   brk(void *addr);

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

    // total bytes needed to be allocated
    uint64_t total = (uint64_t)size + sizeof(struct header);
    char *current_break = (char *)sbrk(0);              // fixed: sbrk now declared

    // current bytes used
    // heap_start declared in allocation step at allocopt
    uint64_t bytes_used = current_break - (char *)heap_start;

    // request memory from free List
    struct header *current = head;
    struct header *previous = NULL;
    struct header *chosen_pos = NULL;
    struct header *chosen_prev_pos = NULL;

    // looping through list to check free blocks
    while (current != NULL) {
        if (current->size >= total) {
            if (algo == FIRST_FIT) {
                chosen_pos = current;
                chosen_prev_pos = previous;
                break;
            } else if (algo == BEST_FIT) {
                if (chosen_pos == NULL || current->size < chosen_pos->size) {
                    chosen_pos = current;
                    chosen_prev_pos = previous;
                }
            } else // WORST_FIT
            {
                if (chosen_pos == NULL || current->size > chosen_pos->size) {
                    chosen_pos = current;
                    chosen_prev_pos = previous;
                }
            }
        }
        previous = current;
        current = current->next;
    }

    // grow the heap if empty
    if (chosen_pos == NULL) {
        if (bytes_used + INCREMENT > (uint64_t)heap_limit) {
            return NULL;
        }

        // request increment defined size with sbrk
        uint64_t requested_size = INCREMENT;

        // must be multiple of INCREMENT
        while (requested_size < total) {
            requested_size += INCREMENT;
        }

        struct header *new_block = (struct header *)sbrk((intptr_t)requested_size); // fixed: cast + declared

        // if invalid then return null
        if (new_block == (void *)-1) {
            return NULL;
        }

        // allocate in list
        new_block->size = requested_size;
        new_block->next = head;
        head = new_block;

        chosen_pos = new_block;
        chosen_prev_pos = NULL;
    }

    // remove from free list
    if (chosen_prev_pos == NULL) {
        head = chosen_pos->next;
    } else {
        chosen_prev_pos->next = chosen_pos->next;
    }

    // if large enough split into blocks
    uint64_t remainder = chosen_pos->size - total;
    uint64_t header_size = sizeof(struct header);

    if (remainder > header_size) {
        struct header *left_over = (struct header *)((char *)chosen_pos + total);

        left_over->size = remainder;
        left_over->next = head;
        head = left_over;
        chosen_pos->size = total;
    }

    chosen_pos->next = NULL;

    return (void *)(chosen_pos + 1);
}

void allocopt(enum algs alg, int limit) {
    // choose program (FIRST_FIT, BEST_FIT, WORST_FIT)
    algo = alg;

    // max heap size defined
    heap_limit = limit;

    // check if heap has been used before
    if (heap_start != NULL) {
        // move address and discard all prev address
        brk(heap_start);                    // fixed: brk now declared
    }

    // save current break point, without moving it
    heap_start = sbrk(0);                   // fixed: sbrk now declared

    // reset free block list.
    head = NULL;
}

void dealloc(void *ptr) {

    if (ptr == NULL) {
        return;
    }

    // compiler method of pointer arithmetic, so i don't need to calculate byte
    // increments.
    struct header *block = (struct header *)ptr - 1;

    struct header *curr = head;
    struct header *prev = NULL;

    // coalesce implementation to merge blocks.
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

            // check for other merges after expansion
            curr = head;
            prev = NULL;
            continue;
        }
        // case 2: if current is after freed block
        else if (block_end == current_pos) {
            // remove curr from list
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                head = curr->next;
            }

            // merge and expand current
            block->size += curr->size;

            // resume search after expansion
            curr = head;
            prev = NULL;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
    // complete the coalesced block
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
        uint64_t available_space = curr->size - sizeof(struct header);

        info.free_size += available_space;
        info.free_chunks++;

        // if only one space available
        if (info.free_chunks == 1) {
            info.largest_free_chunk_size = available_space;
            info.smallest_free_chunk_size = available_space;
        } else {
            // assign space to larger chunk
            if (available_space > info.largest_free_chunk_size) {
                info.largest_free_chunk_size = available_space;
            }
            // assign space to smaller chunk
            if (available_space < info.smallest_free_chunk_size) {
                info.smallest_free_chunk_size = available_space;
            }
        }

        curr = curr->next;
    }

    return info;
}
