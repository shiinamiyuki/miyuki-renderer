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

#include <miyuki.foundation/detail/object-funcs.h>
#include <miyuki.renderer/scene.h>
#include "accelerators/sahbvh.h"
#include "accelerators/embree-backend.h"
#include "lights/arealight.h"
#include <miyuki.foundation/property.hpp>

namespace miyuki::core {
    void Scene::preprocess() {
#ifdef MYK_USE_EMBREE
        accelerator = std::make_shared<EmbreeAccelerator>();
#else
        accelerator = std::make_shared<BVHAccelerator>();
#endif
        auto setLight = [=](MeshTriangle *triangle) {
            auto mat = triangle->getMaterial();
            if (mat && mat->markAsLight && mat->emission && mat->emissionStrength) {
                auto light = std::make_shared<AreaLight>();
                triangle->light = light.get();
                light->setTriangle(triangle);
                lights.emplace_back(light);
            }
        };
        struct PreprocessVisitor : public PropertyVisitor {
            void visit(ObjectProperty *aProperty) override {
                PropertyVisitor::visit(aProperty);
                if (aProperty->getRef()) {
                    aProperty->getRef()->preprocess();
                    aProperty->getRef()->accept(this);
                }
            }
        };

        PreprocessVisitor visitor;
        for (auto &i : meshes) {
            i->preprocess();
            i->accept(&visitor);
            i->foreach(setLight);
        }
        accelerator->build(*this);
    }

    bool Scene::intersect(const miyuki::core::Ray &ray, miyuki::core::Intersection &isct) {
        rayCounter++;
        if (accelerator->intersect(ray, isct)) {
            isct.Ns = isct.shape->normalAt(isct.uv);
            isct.material = isct.shape->getMaterial();
            isct.wo = -ray.d;
            isct.computeLocalFrame();
            return true;
        }
        return false;
    }
} // namespace miyuki::core