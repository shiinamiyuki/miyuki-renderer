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

#ifndef MIYUKIRENDERER_SAHBVH_H
#define MIYUKIRENDERER_SAHBVH_H

#include <miyuki.renderer/accelerator.h>
#include <miyuki.foundation/interfaces.h>
#include <miyuki.renderer/mesh.h>


namespace miyuki::core {

    class BVHAccelerator final : public Accelerator {
        class BVHAcceleratorInternal;

        std::vector<BVHAcceleratorInternal *> internal;
    public:
        MYK_DECL_CLASS(BVHAccelerator, "BVHAccelerator", interface = "BVHAccelerator")

        void build(Scene &scene) override;

        bool intersect(const Ray &ray, Intersection &isct) override;

        bool occlude(const Ray & ray)override;

        ~BVHAccelerator();
    };
}
#endif //MIYUKIRENDERER_SAHBVH_H
