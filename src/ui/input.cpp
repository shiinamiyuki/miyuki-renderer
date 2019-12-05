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

#include <miyuki.renderer/ui/input.h>
#include <miyuki.renderer/ui/myk-ui.h>
#include <miyuki.foundation/log.hpp>
#include <imgui.h>

namespace miyuki ::ui {
//        std::optional<Transform> GetInput(const std::string&prompt, const Transform& initial) {
//            Text().name(prompt).show();
//            auto r1 = GetInput("translation", initial.translation);
//            auto r2 = GetInput("rotation", initial.rotation);
//            auto r3 = GetInput("scale", initial.scale);
//            if (r1.has_value() || r2.has_value() || r3.has_value()) {
//                return Transform(r1.value(), r2.value(), r3.value());
//            }
//            return {};
//        }
    std::optional<bool> GetInput(const std::string &prompt, bool initial) {
        bool temp = initial;
        if (ImGui::Checkbox(prompt.c_str(), &temp)) {
            return std::optional<bool>(temp);
        } else {
            return {};
        }
    }

    std::optional<Point2i> GetInput(const std::string &prompt, Point2i initial) {
        auto temp = initial;
        if (ImGui::InputInt2(prompt.c_str(), &temp[0], ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::optional<Point2i>(temp);
        }
        return {};
    }

    std::optional<size_t> GetInput(const std::string &prompt, size_t initial) {
        int temp = initial;
        if (ImGui::InputInt(prompt.c_str(), &temp, ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::optional<size_t>(temp);
        } else {
            return {};
        }
    }

    std::optional<int> GetInput(const std::string &prompt, int initial) {
        int temp = initial;
        if (ImGui::InputInt(prompt.c_str(), &temp, ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::optional<int>(temp);
        } else {
            return {};
        }
    }

    std::optional<Float> GetInput(const std::string &prompt, Float initial) {
        Float temp = initial;
        if (ImGui::InputFloat(prompt.c_str(), &temp, 0.01, 0.1, "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            return temp;
        } else {
            return {};
        }
    }

    std::optional<Point2f> GetInput(const std::string &prompt, Point2f initial) {
        auto temp = initial;
        if (ImGui::InputFloat2(prompt.c_str(), &temp[0], "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::optional<Point2f>(temp);
        }
        return {};
    }


    std::optional<Vec3f> GetInput(const std::string &prompt, Vec3f initial) {
        auto temp = initial;
        if (ImGui::InputFloat3(prompt.c_str(), &temp[0], "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::optional<Vec3f>(temp);
        }
        return {};
    }

    std::optional<core::Spectrum> GetInput(const std::string &prompt, core::Spectrum initial) {
        auto temp = initial;
        std::optional<core::Spectrum> result;
        if (ImGui::ColorPicker3(prompt.c_str(), &temp[0], ImGuiInputTextFlags_EnterReturnsTrue)) {
            result = std::optional<core::Spectrum>(temp);
        }
        if (ImGui::InputFloat3(prompt.c_str(), &temp[0], "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            result = std::optional<core::Spectrum>(temp);
        }
        return result;
    }

    std::optional<Float> GetFloatClamped(const std::string &prompt, Float initial, Float minVal, Float maxVal) {
        Float temp = initial;
        if (ImGui::SliderFloat(prompt.c_str(), &temp, minVal, maxVal)) {
            return std::optional<Float>(temp);
        }
        return {};
    }

    std::optional<std::string> GetInput(const std::string &prompt, const std::string &initial) {
        std::vector<char> buffer(4096);
        for (int i = 0; i < initial.size(); i++) {
            buffer[i] = initial[i];
        }
        if (ImGui::InputText(prompt.c_str(), &buffer[0], buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (buffer.back() != 0) {
                log::log("buffer.back() != 0\n");
            }
            return std::optional<std::string>(&buffer[0]);
        }
        return {};
    }
}
