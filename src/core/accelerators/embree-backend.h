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

#ifndef MIYUKIRENDERER_EMBREE_BACKEND_H
#define MIYUKIRENDERER_EMBREE_BACKEND_H

#include <miyuki.renderer/accelerator.h>
#include <miyuki.foundation/interfaces.h>
#include <miyuki.foundation/noncopyable.hpp>
#include <miyuki.renderer/ray.h>

namespace miyuki::core {
    class EmbreeAccelerator final : public Accelerator, private NonCopyable{
        class Impl;
        Impl *impl = nullptr;
    public:
        EmbreeAccelerator();
        MYK_DECL_CLASS(EmbreeAccelerator, "EmbreeAccelerator", interface = "Accelerator")

        void build(Scene &scene) override;

        bool intersect(const Ray &ray, Intersection &isct) override;

        bool4 intersect4(const Ray4 &ray, Intersection4 &isct) override;

        bool8 intersect8(const Ray8 &ray, Intersection8 &isct) override;

        ~EmbreeAccelerator();

    };

}
#endif //MIYUKIRENDERER_EMBREE_BACKEND_H
