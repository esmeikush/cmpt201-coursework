// alloc.c
#include "alloc.h"
#include <unistd.h>   // sbrk()

// ─── Global state ─────────────────────────────────────────────────────────────

static struct header *free_list  = NULL;   // Head of the free block list
static int            heap_limit = 0;      // Max bytes we can sbrk(); 0 = no limit
static int            total_sbrk = 0;      // How many bytes we've sbrk()'d so far
static enum algs      cur_alg    = FIRST_FIT;

// ─── allocopt() ───────────────────────────────────────────────────────────────

void allocopt(enum algs alg, int limit) {
    cur_alg    = alg;
    heap_limit = limit;
}

// ─── grow_heap() ──────────────────────────────────────────────────────────────
// Ask the OS for INCREMENT more bytes.
// Treat the new memory as one big free block, add it to the front of the list.

static int grow_heap(void) {
    // Check we won't go over the limit
    if (heap_limit > 0 && total_sbrk + INCREMENT > heap_limit) {
        return -1;
    }

    // Ask the OS for more heap space
    void *new_memory = sbrk(INCREMENT);
    if (new_memory == (void *)-1) {
        return -1;   // sbrk() failed
    }

    total_sbrk += INCREMENT;

    // Turn the new memory into a free block
    struct header *new_block = (struct header *)new_memory;
    new_block->size = INCREMENT;

    // Add it to the front of the free list
    new_block->next = free_list;
    free_list = new_block;

    return 0;
}

// ─── find_block() ─────────────────────────────────────────────────────────────
// Walk the free list and pick a block based on the current algorithm.
// Remove the chosen block from the list and return it.

static struct header *find_block(uint64_t needed) {
    struct header *prev      = NULL;
    struct header *curr      = free_list;
    struct header *best      = NULL;
    struct header *best_prev = NULL;

    while (curr != NULL) {
        if (curr->size >= needed) {
            // This block is big enough — check if it's better than our current best
            if (cur_alg == FIRST_FIT) {
                // Just take the first one we find, no need to keep looking
                best      = curr;
                best_prev = prev;
                break;

            } else if (cur_alg == BEST_FIT) {
                // We want the SMALLEST block that still fits
                if (best == NULL || curr->size < best->size) {
                    best      = curr;
                    best_prev = prev;
                }

            } else {
                // WORST_FIT: we want the LARGEST block
                if (best == NULL || curr->size > best->size) {
                    best      = curr;
                    best_prev = prev;
                }
            }
        }

        prev = curr;
        curr = curr->next;
    }

    // If nothing was found, return NULL
    if (best == NULL) {
        return NULL;
    }

    // Remove the chosen block from the free list
    if (best_prev == NULL) {
        // It was the head of the list
        free_list = best->next;
    } else {
        // Skip over it
        best_prev->next = best->next;
    }
    best->next = NULL;

    return best;
}

// ─── alloc() ──────────────────────────────────────────────────────────────────

void *alloc(int size) {
    if (size <= 0) {
        return NULL;
    }

    // Total space needed = the header + the actual data the user wants
    uint64_t needed = (uint64_t)size + sizeof(struct header);

    // Try to find a free block
    struct header *block = find_block(needed);

    // No free block found — grow the heap and try once more
    if (block == NULL) {
        if (grow_heap() < 0) {
            return NULL;   // At the limit or sbrk() failed
        }
        block = find_block(needed);
        if (block == NULL) {
            return NULL;   // Size requested is larger than INCREMENT
        }
    }

    // --- Splitting ---
    // If the block is much bigger than we need, split it:
    //   [ header | user data ] [ header | leftover free space ]
    // But if the leftover is too small to even hold a header, don't bother
    // splitting — just give the user the whole block.

    uint64_t leftover = block->size - needed;

    if (leftover > sizeof(struct header)) {
        // Carve off the leftover as a new free block
        struct header *free_block = (struct header *)((char *)block + needed);
        free_block->size = leftover;

        // Put the new free block at the front of the free list
        free_block->next = free_list;
        free_list = free_block;

        // Shrink the allocated block to exactly what was needed
        block->size = needed;
    }
    // else: no split, hand over the whole block

    // Return a pointer to the memory just AFTER the header
    // That's the usable memory the user cares about
    return (void *)(block + 1);
}

// ─── dealloc() ────────────────────────────────────────────────────────────────

void dealloc(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    // Step back by one header to get back to the block's header
    struct header *block = (struct header *)ptr - 1;

    // Put it at the front of the free list
    block->next = free_list;
    free_list   = block;
}

// ─── allocinfo() ──────────────────────────────────────────────────────────────
// Walk the free list and collect stats.

struct allocinfo allocinfo(void) {
    struct allocinfo info;
    info.free_size               = 0;
    info.free_chunks             = 0;
    info.largest_free_chunk_size = 0;
    info.smallest_free_chunk_size = 0;

    struct header *curr = free_list;

    while (curr != NULL) {
        info.free_size   += curr->size;
        info.free_chunks += 1;

        // First block — set both min and max to this size
        if (info.free_chunks == 1) {
            info.largest_free_chunk_size  = curr->size;
            info.smallest_free_chunk_size = curr->size;
        } else {
            if (curr->size > info.largest_free_chunk_size) {
                info.largest_free_chunk_size = curr->size;
            }
            if (curr->size < info.smallest_free_chunk_size) {
                info.smallest_free_chunk_size = curr->size;
            }
        }

        curr = curr->next;
    }

    return info;
}
