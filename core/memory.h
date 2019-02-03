//
// Created by Shiina Miyuki on 2019/1/31.
//

#ifndef MIYUKI_MEMORY_H
#define MIYUKI_MEMORY_H

#include "util.h"
#include <list>

namespace Miyuki {
#define ALLOCA(TYPE, COUNT) (TYPE *)alloca((COUNT) * sizeof(TYPE))
#ifndef MIYUKI_L1_CACHE_LINE_SIZE
#define MIYUKI_L1_CACHE_LINE_SIZE 64
#endif

    void *_allocAligned(size_t size);

    template<typename T>
    T *allocAligned(size_t count) {
        return (T *) _allocAligned(count * sizeof(T));
    }

    class MemoryArena {

    };
}
#endif //MIYUKI_MEMORY_H
