//
// Created by Shiina Miyuki on 2019/1/31.
//

#include "memory.h"

using namespace Miyuki;

void *_allocAligned(size_t size) {
#if defined(MIYUKI_ON_WINDOWS)
    return _aligned_malloc(size, MIYUKI_L1_CACHE_LINE_SIZE);
#elif defined (MIYUKI_IS_OPENBSD) || defined(MIYUKI_IS_OSX)
    void *ptr;
    if (posix_memalign(&ptr, MIYUKI_L1_CACHE_LINE_SIZE, size) != 0)
        ptr = nullptr;
    return ptr;
#else
    return memalign(MIYUKI_L1_CACHE_LINE_SIZE, size);
#endif
}