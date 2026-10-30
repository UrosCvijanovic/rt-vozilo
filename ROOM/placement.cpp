// placement.cpp - manually written/modified per Memory Placement chapter
// (manual lines 888-940).
//
// RADNA KOPIJA (rt-vozilo). Original alata je u ROOM/upstream/placement.cpp (17.3).
// Izmene u odnosu na upstream (tabela-aktera.md, otvoreno pitanje 8):
//  1. stackRegionSize 4 KB -> 8 KB. StackMem<N> zauzima N + 8 B (16.3), pa
//     4 aktera x StackMem<1024> = 4128 B > 4096 B: cetvrti stek (Plovilo, po
//     redosledu build()) dobio bi nullptr. Ceo model: 9 x 1032 = 9288 B.
//  2. Iscrpljenje regiona je GLASNA greska: ispis i zaustavljanje u
//     regionExhausted(), umesto tihog nullptr (alokator iz lib.h vraca nullptr,
//     a konstruktor bi zatim pisao na adresu 0).
//  3. Brojanje zauzeca po regionu i ispis (placementReport) za dimenzionisanje.
//
// Sve alokacije se desavaju u userMain (create(), build()), dakle privilegovano,
// pa ispis preko semihostinga ovde radi (16.5).

#include <cstdio>

#include "placement.h"
#include "lib.h"

namespace {

constexpr size_t anyRegionSize = 1 * 1024;
constexpr size_t taskRegionSize = 1 * 1024;
constexpr size_t stackRegionSize = 12 * 1024; // upstream: 4 * 1024
constexpr size_t actorsRegionSize = 4 * 1024;
constexpr size_t localBuffersRegionSize = 4 * 1024;
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

// ---- rt-vozilo: zauzece po regionu (indeks = vrednost MemRegion) ----
constexpr unsigned regionCount = 8;

constexpr size_t regionCapacity[regionCount] = {
    anyRegionSize, taskRegionSize, stackRegionSize, actorsRegionSize,
    localBuffersRegionSize, sharedBuffersRegionSize, fastRAMRegionSize, slowRAMRegionSize
};

const char* const regionName[regionCount] = {
    "any", "tasks", "stacks", "actors",
    "localBuffers", "sharedBuffers", "fastRAM", "slowRAM"
};

// Zbir trazenih velicina. Tacno za regione sa Allocator-om (bez zaokruzivanja);
// sharedBuffers zaokruzuje na 8 B, ali se na jednom jezgru ne koristi.
size_t regionUsed[regionCount] = {};

unsigned regionIndex(MemRegion r) {
    const unsigned i = static_cast<unsigned>(r);
    return (i < regionCount) ? i : static_cast<unsigned>(any);   // nepoznato -> any, kao default u switch-u
}

} // namespace

// Van anonimnog prostora imena: function breakpoint "regionExhausted" (14.6).
__attribute__((noinline, noreturn))
void regionExhausted(MemRegion region, size_t size) {
    const unsigned i = regionIndex(region);
    printf("PLACEMENT: region %s ISCRPLJEN: trazeno %u B, zauzeto %u od %u B\n",
           regionName[i], static_cast<unsigned>(size),
           static_cast<unsigned>(regionUsed[i]), static_cast<unsigned>(regionCapacity[i]));
    for (;;) { }
}

void placementReport() {
    for (unsigned i = 0; i < regionCount; ++i) {
        printf("PLACEMENT: %-13s %5u / %5u B\n", regionName[i],
               static_cast<unsigned>(regionUsed[i]), static_cast<unsigned>(regionCapacity[i]));
    }
}

void resetSharedBuffersAllocator() {
    sharedBuffersAlloc.reset();
    regionUsed[sharedBuffers] = 0;
}

void* operator new(size_t size, MemRegion region) {
    void* p = nullptr;
    switch (region) {
        case tasks:          p = taskAlloc.malloc(size);          break;
        case stacks:         p = stackAlloc.malloc(size);         break;
        case actors:         p = actorsAlloc.malloc(size);        break;
        case localBuffers:   p = localBuffersAlloc.malloc(size);  break;
        case sharedBuffers:  p = sharedBuffersAlloc.malloc(size); break;
        case fastRAM:        p = fastRAMAlloc.malloc(size);       break;
        case slowRAM:        p = slowRAMAlloc.malloc(size);       break;
        case any:
        default:             p = alloc0.malloc(size);             break;
    }
    if (p == nullptr) {
        regionExhausted(region, size);   // ne vraca se: nullptr nikad ne izlazi odavde
    }
    regionUsed[regionIndex(region)] += size;
    return p;
}

void* operator new[](size_t size, MemRegion region) {
    return operator new(size, region);
}
