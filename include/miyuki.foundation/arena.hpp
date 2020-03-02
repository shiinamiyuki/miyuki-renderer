// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MIYUKIRENDERER_ARENA_HPP
#define MIYUKIRENDERER_ARENA_HPP

#include <algorithm>
#include <list>
#include <cstdint>

namespace miyuki {
    class Arena {
        static constexpr size_t align16(size_t x) {
            return (x + 15ULL) & (~15ULL);
        }

        struct Block {
            size_t size;
            uint8_t *data;

            Block(uint8_t *data, size_t size) : size(size), data(data) {
//                log::log("???\n");
            }

            ~Block() = default;
        };

        std::list<Block> availableBlocks, usedBlocks;

        size_t currentBlockPos = 0;
        Block currentBlock;
    public:
        Arena():currentBlock(new uint8_t[262144], 262144) {
        }

        template<class T>
        T *allocN(size_t count) {
            typename std::list<Block>::iterator iter;
            auto allocSize = sizeof(T) * count;

            uint8_t  * p = nullptr;
            if(currentBlockPos + allocSize > currentBlock.size){
                usedBlocks.emplace_front(currentBlock);
                currentBlockPos = 0;
                for(iter = availableBlocks.begin();iter!=availableBlocks.end();iter++){
                    if(iter->size >= allocSize){
                        currentBlockPos = allocSize;
                        currentBlock = *iter;
                        availableBlocks.erase(iter);
                        break;
                    }
                }
                if(iter == availableBlocks.end()) {
                    auto sz = std::max<size_t>(allocSize, 262144ull);
                    currentBlock = Block(new uint8_t[sz], sz);
                }
            }
            p = currentBlock.data + currentBlockPos;
            currentBlockPos += allocSize;
            if constexpr (!std::is_trivially_constructible_v<T>) {
                for (int i = 0; i < count; i++) {
                    new(p + i * sizeof(T)) T();
                }
            }
            return reinterpret_cast<T *>(p);
        }

        template<typename T>
        T *alloc() {
            return allocN<T>(1);
        }

        void reset() {
            currentBlockPos = 0;
            availableBlocks.splice(availableBlocks.begin(), usedBlocks);
        }

        ~Arena() {
            delete [] currentBlock.data;
            for (auto i: availableBlocks) {
                delete[]i.data;
            }
            for (auto i:usedBlocks) {
                delete[] i.data;
            }
        }
    };
}
#endif //MIYUKIRENDERER_ARENA_HPP
