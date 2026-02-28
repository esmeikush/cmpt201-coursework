#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "alloc.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

static struct header *head = NULL;
static int heap_limit = 0;
static void *heap_start = NULL;
static enum algs algo = FIRST_FIT;

void *alloc(int size) {
    if (size <= 0) return NULL;

    // Use your original total calculation
    uint64_t total = (uint64_t)size + sizeof(struct header);
    
    char *current_break = (char *)sbrk(0);
    // Use uintptr_t for safe pointer subtraction to avoid warnings
    uint64_t bytes_used = (uintptr_t)current_break - (uintptr_t)heap_start;

    struct header *curr = head;
    struct header *prev = NULL;
    struct header *chosen = NULL;
    struct header *chosen_prev = NULL;

    while (curr != NULL) {
        if (curr->size >= total) {
            if (algo == FIRST_FIT) {
                chosen = curr;
                chosen_prev = prev;
                break;
            } else if (algo == BEST_FIT) {
                if (chosen == NULL || curr->size < chosen->size) {
                    chosen = curr;
                    chosen_prev = prev;
                }
            } else {
                if (chosen == NULL || curr->size > chosen->size) {
                    chosen = curr;
                    chosen_prev = prev;
                }
            }
        }
        prev = curr;
        curr = curr->next;
    }

    if (chosen == NULL) {
        uint64_t requested_size = INCREMENT;
        while (requested_size < total) {
            requested_size += INCREMENT;
        }

        if (bytes_used + requested_size > (uint64_t)heap_limit) {
            return NULL;
        }

        struct header *new_block = (struct header *)sbrk(requested_size);
        if (new_block == (void *)-1) return NULL;

        new_block->size = requested_size;
        new_block->next = head;
        head = new_block;

        chosen = new_block;
        chosen_prev = NULL;
    }

    if (chosen_prev == NULL) {
        head = chosen->next;
    } else {
        chosen_prev->next = chosen->next;
    }

    uint64_t remainder = chosen->size - total;
    if (remainder > sizeof(struct header)) {
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
    algo = alg;
    heap_limit = limit;

    // CRITICAL FIX: Only set heap_start once.
    // This ensures 'base' in the test matches your 'heap_start'.
    if (heap_start == NULL) {
        heap_start = sbrk(0);
    } else {
        brk(heap_start);
    }

    head = NULL;
}

void dealloc(void *ptr) {
    if (ptr == NULL) return;
    struct header *block = (struct header *)ptr - 1;
    struct header *curr = head;
    struct header *prev = NULL;

    while (curr != NULL) {
        char *current_pos = (char *)curr;
        char *current_end = current_pos + curr->size;
        char *block_pos = (char *)block;
        char *block_end = block_pos + block->size;

        if (current_end == block_pos) {
            if (prev != NULL) prev->next = curr->next;
            else head = curr->next;
            curr->size += block->size;
            block = curr;
            curr = head;
            prev = NULL;
            continue;
        } else if (block_end == current_pos) {
            if (prev != NULL) prev->next = curr->next;
            else head = curr->next;
            block->size += curr->size;
            curr = head;
            prev = NULL;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
    block->next = head;
    head = block;
}

struct allocinfo allocinfo(void) {
    struct allocinfo info = {0, 0, 0, 0};
    struct header *curr = head;
    while (curr != NULL) {
        int available = curr->size - sizeof(struct header);
        info.free_size += available;
        info.free_chunks++;
        if (info.free_chunks == 1) {
            info.largest_free_chunk_size = available;
            info.smallest_free_chunk_size = available;
        } else {
            if (available > info.largest_free_chunk_size) info.largest_free_chunk_size = available;
            if (available < info.smallest_free_chunk_size) info.smallest_free_chunk_size = available;
        }
        curr = curr->next;
    }
    return info;
}
