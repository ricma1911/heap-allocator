#ifndef ALLOCATOR_DEBUG_H
#define ALLOCATOR_DEBUG_H

typedef enum {
    AL_DEBUG_OK,
    AL_DEBUG_INVALID_FREE_VALUE,
    AL_DEBUG_INVALID_TOTAL_SIZE,
    AL_DEBUG_TWO_FREE_BLOCKS_SEQUENCE,
    AL_DEBUG_INVALID_NEXT,
    AL_DEBUG_INVALID_SIZE
} debug_status;

debug_status allocator_validate();

#endif