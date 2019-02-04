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

    void freeAligned(void *);

    template<typename T>
    T *allocAligned(size_t count) {
        return (T *) _allocAligned(count * sizeof(T));
    }

    class MemoryArena {
    private:
        const size_t blockSize;
        size_t currentBlockPos = 0, currentAllocSize = 0;
        uint8_t *currentBlock = nullptr;
        std::list<std::pair<size_t, uint8_t * >> usedBlocks, availableBlocks;
    public:
        MemoryArena(size_t blockSize = 262144) : blockSize(blockSize) {}

        void *alloc(size_t bytes);

        template<typename T>
        T *alloc(size_t n = 1, bool runConstructor = true) {
            T *ret = (T *) alloc(n * sizeof(T));
            if (runConstructor) {
                for (auto i = 0; i < n; i++) {
                    new(&ret[i])T();
                }
            }
            return ret;
        }

        ~MemoryArena();

        void reset();
    };

    class ConcurrentMemoryArena : public MemoryArena {
    private:
        std::mutex mutex;
    public:
        ConcurrentMemoryArena(size_t blockSize = 262144) :MemoryArena(blockSize){}
        void *alloc(size_t bytes);
        void reset();
    };
}
#endif //MIYUKI_MEMORY_H
