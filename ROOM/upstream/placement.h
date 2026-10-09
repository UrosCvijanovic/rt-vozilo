// placement.h - manually written/modified per Memory Placement chapter
// (manual lines 871-884).

#ifndef HDS_PLACEMENT_H
#define HDS_PLACEMENT_H

#include <cstddef>

enum MemRegion : char {
    any,
    tasks,
    stacks,
    actors,
    localBuffers,
    sharedBuffers,
    fastRAM,
    slowRAM
};

/// Reset the shared-buffer allocator cursor before each actor-tree build.
/// Both cores must run builds in the same order so shared queue addresses match.
void resetSharedBuffersAllocator();

void* operator new(std::size_t size, MemRegion region);
void* operator new[](std::size_t size, MemRegion region);

#endif
