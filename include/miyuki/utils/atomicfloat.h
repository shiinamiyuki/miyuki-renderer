//
// Created by Shiina Miyuki on 2019/2/18.
//

#ifndef MIYUKI_ATOMICFLOAT_H
#define MIYUKI_ATOMICFLOAT_H

#include "miyuki.h"

namespace Miyuki {
    inline uint32_t floatToBits(float f) {
        uint32_t ui;
        memcpy(&ui, &f, sizeof(float));
        return ui;
    }

    inline float bitsToFloat(uint32_t ui) {
        float f;
        memcpy(&f, &ui, sizeof(uint32_t));
        return f;
    }

    class AtomicFloat {
    public:
        AtomicFloat(Float v = 0) { bits = floatToBits(v); }

        AtomicFloat(const AtomicFloat &rhs) {
            bits.store(rhs.bits.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }

        operator Float() const { return bitsToFloat(bits); }

        Float operator=(Float v) {
            bits = floatToBits(v);
            return v;
        }
        AtomicFloat& operator = (const AtomicFloat & rhs){
            bits.store(rhs.bits.load(std::memory_order_relaxed), std::memory_order_relaxed);
            return *this;
        }
        void add(Float v) {
            uint32_t oldBits = bits, newBits;
            do {
                newBits = floatToBits(bitsToFloat(oldBits) + v);
            } while (!bits.compare_exchange_weak(oldBits, newBits));
        }

        void store(Float v){
            bits.store(floatToBits(v), std::memory_order_relaxed);
        }
    private:
        std::atomic<uint32_t> bits;


    };
}
#endif //MIYUKI_ATOMICFLOAT_H
