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

#ifndef MIYUKIRENDERER_CAMERA_H
#define MIYUKIRENDERER_CAMERA_H

#include <miyuki.renderer/interfaces.h>
#include <miyuki.renderer/ray.h>
#include <miyuki.renderer/interfaces.h>

namespace miyuki::core {
    struct CameraSample {
        Point2f pLens;
        Point2i pFilm;
        Ray ray;
    };

    class Camera : public serialize::Serializable {
    public:
        MYK_INTERFACE(Camera, "Camera")


        virtual const Transform &getTransform() const = 0;

        virtual void generateRay(const Point2f &u1,
                                 const Point2f &u2,
                                 const Point2i &raster,
                                 Point2i filmDimension,
                                 CameraSample &sample) const = 0;

        virtual void preprocess(){}
    };
}
#endif //MIYUKIRENDERER_CAMERA_H
