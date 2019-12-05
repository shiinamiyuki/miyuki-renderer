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

#ifndef MIYUKIRENDERER_INPUT_H
#define MIYUKIRENDERER_INPUT_H

#include <optional>
#include <miyuki.foundation/spectrum.h>

namespace miyuki::ui {
    std::optional<Transform> GetInput(const std::string &, const Transform &initial);

    std::optional<bool> GetInput(const std::string &, bool initial);

    std::optional<int> GetInput(const std::string &, int initial);

    std::optional<Float> GetInput(const std::string &, Float initial);

    std::optional<size_t> GetInput(const std::string &, size_t initial);

    std::optional<Point2i> GetInput(const std::string &prompt, Point2i initial);

    std::optional<Point2f> GetInput(const std::string &prompt, Point2f initial);

    std::optional<Vec3f> GetInput(const std::string &, Vec3f initial);

    std::optional<core::Spectrum> GetInput(const std::string &, core::Spectrum initial);

    std::optional<Float> GetFloatClamped(const std::string &, Float initial, Float minVal, Float maxVal);

    std::optional<std::string> GetInput(const std::string &, const std::string &);
}
#endif //MIYUKIRENDERER_INPUT_H
