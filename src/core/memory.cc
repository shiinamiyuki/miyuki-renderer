//
// Created by Shiina Miyuki on 2019/1/31.
//

#include <core/memory.h>
#include <utils/thread.h>

#ifdef MIYUKI_ON_WINDOWS
#define MIYUKI_HAS_ALIGNED_MALLOC
#else
#endif
using namespace Miyuki;
#ifdef MIYUKI_ON_WINDOWS
#define MIYUKI_HAS_ALIGNED_MALLOC
#else
#endif

void *Miyuki::_allocAligned(size_t size) {
#if defined(MIYUKI_HAS_ALIGNED_MALLOC)
    return _aligned_malloc(size, MIYUKI_L1_CACHE_LINE_SIZE);
#elif defined (MIYUKI_HAS_POSIX_MEMALIGN)
    void *ptr;
    if (posix_memalign(&ptr, MIYUKI_L1_CACHE_LINE_SIZE, size) != 0)
        ptr = nullptr;
    return ptr;
#else
    return memalign(MIYUKI_L1_CACHE_LINE_SIZE, size);
#endif
}

void Miyuki::freeAligned(void *ptr) {
    if (!ptr) return;
#if defined(MIYUKI_HAS_ALIGNED_MALLOC)
    _aligned_free(ptr);
#else
    free(ptr);
#endif

}

MemoryArena::~MemoryArena() {
    freeAligned(currentBlock);
    for (auto &block:usedBlocks)freeAligned(block.second);
    for (auto &block:availableBlocks)freeAligned(block.second);
}

void *MemoryArena::alloc(size_t bytes) {
    bytes = ((bytes + 15) & (~15));
    if (currentBlockPos + bytes >= currentAllocSize) {
        if (currentBlock) {
            usedBlocks.emplace_back(currentAllocSize, currentBlock);
            currentBlock = nullptr;
        }
        for (auto iter = availableBlocks.begin(); iter != availableBlocks.end(); iter++) {
            if (iter->first >= bytes) {
                currentAllocSize = iter->first;
                currentBlock = iter->second;
                availableBlocks.erase(iter);
                break;
            }
        }
        if (!currentBlock) { // no available blocks
            currentAllocSize = std::max(bytes, blockSize);
            currentBlock = allocAligned<uint8_t>(currentAllocSize);
        }
        currentBlockPos = 0;
    }
    void *ret = currentBlock + currentBlockPos;
    currentBlockPos += bytes;
    return ret;
}

