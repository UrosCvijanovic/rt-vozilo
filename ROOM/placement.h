// placement.h - manually written/modified per Memory Placement chapter
// (manual lines 871-884).
//
// RADNA KOPIJA (rt-vozilo). Original alata je u ROOM/upstream/placement.h (17.3).
// Izmena u odnosu na upstream: deklaracija placementReport().

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

/// rt-vozilo: ispis zauzeca svakog regiona (zauzeto / kapacitet, u bajtovima).
/// Poziva se iz userMain posle root->build(), kad su sve alokacije gotove.
/// Ispis ide preko semihostinga, pa samo iz privilegovanog koda (16.5).
void placementReport();

#endif
