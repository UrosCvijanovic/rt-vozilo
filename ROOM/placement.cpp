// placement.cpp - manually written/modified per Memory Placement chapter
// (manual lines 888-940).

#include "placement.h"
#include "lib.h"

namespace {

constexpr size_t anyRegionSize = 16 * 1024;
constexpr size_t taskRegionSize = 4 * 1024;
constexpr size_t stackRegionSize = 4 * 1024;
constexpr size_t actorsRegionSize = 4 * 1024;
constexpr size_t localBuffersRegionSize = 2 * 1024;
constexpr size_t sharedBuffersRegionSize = 2 * 1024;
constexpr size_t fastRAMRegionSize = 1 * 1024;
constexpr size_t slowRAMRegionSize = 2 * 1024;

Allocator<anyRegionSize> alloc0;
Allocator<taskRegionSize> taskAlloc;
Allocator<stackRegionSize> stackAlloc;
Allocator<actorsRegionSize> actorsAlloc;
Allocator<localBuffersRegionSize> localBuffersAlloc;
Allocator<fastRAMRegionSize> fastRAMAlloc;
Allocator<slowRAMRegionSize> slowRAMAlloc;

class RegionAllocator {
public:
    RegionAllocator(char* base, size_t capacity) : storage(base), capacity(capacity) {}

    void reset() { used = 0; }

    void* malloc(size_t size) {
        if (size == 0 || storage == nullptr) {
            return nullptr;
        }
        size = (size + 7u) & ~size_t(7u);
        if (used + size > capacity) {
            return nullptr;
        }
        void* ptr = storage + used;
        used += size;
        return ptr;
    }

private:
    char* storage;
    size_t capacity;
    size_t used = 0;
};

#if defined(__ICCARM__)
#pragma location = ".room_shared_buffers"
__root char sharedBuffersBacking[sharedBuffersRegionSize];
#else
char sharedBuffersBacking[sharedBuffersRegionSize]
    __attribute__((section(".room_shared_buffers")));
#endif

RegionAllocator sharedBuffersAlloc(sharedBuffersBacking, sharedBuffersRegionSize);

} // namespace

void resetSharedBuffersAllocator() {
    sharedBuffersAlloc.reset();
}

void* operator new(size_t size, MemRegion region) {
    switch (region) {
        case tasks:          return taskAlloc.malloc(size);
        case stacks:         return stackAlloc.malloc(size);
        case actors:         return actorsAlloc.malloc(size);
        case localBuffers:   return localBuffersAlloc.malloc(size);
        case sharedBuffers:  return sharedBuffersAlloc.malloc(size);
        case fastRAM:        return fastRAMAlloc.malloc(size);
        case slowRAM:        return slowRAMAlloc.malloc(size);
        case any:
        default:      return alloc0.malloc(size);
    }
}

void* operator new[](size_t size, MemRegion region) {
    return operator new(size, region);
}
