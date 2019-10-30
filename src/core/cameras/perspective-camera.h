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

#ifndef MIYUKIRENDERER_PERSPECTIVE_CAMERA_H
#define MIYUKIRENDERER_PERSPECTIVE_CAMERA_H

#include <api/camera.h>
#include <api/serialize.hpp>

namespace miyuki::core {
    class PerspectiveCamera final : public Camera {
        Transform transform, invTransform;
        float fov;
    public:
        PerspectiveCamera() = default;

        PerspectiveCamera(const Vec3f &p1, const Vec3f &p2, Float fov) : fov(fov) {
            transform = Transform(Matrix4::lookAt(p1, p2));
            invTransform = transform.inverse();
        }

        const Transform &getTransform() const override {
            return transform;
        }

    public:
        MYK_DECL_CLASS(PerspectiveCamera, "PerspectiveCamera", interface = "Camera")

        MYK_AUTO_SER(transform, fov)

        void initialize(const json &params) override;

        void generateRay(const Point2f &u1,
                         const Point2f &u2,
                         const Point2i &raster,
                         Point2i filmDimension,
                         CameraSample &sample) const override;
    };

}

#endif //MIYUKIRENDERER_PERSPECTIVE_CAMERA_H
