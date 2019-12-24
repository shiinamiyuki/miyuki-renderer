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

#include "expr-shader.h"
#include <miyuki.foundation/log.hpp>
#include <PerlinNoise.hpp>
#include <memory>
#include <miyuki.foundation/imageloader.h>

static const siv::PerlinNoise perlin(0);

miyuki::core::Spectrum miyuki::core::shading::ExecutionEngine::execute(const miyuki::core::ShadingPoint &sp) {
    auto state = ExecutionState();
    while (state.pc < code.size()) {
        auto &inst = code[state.pc];
        switch (inst.opcode) {
            case Push:
                state.push(inst.operand);
                break;
            case Add: {
                auto B = state.pop();
                state.top() += B;
            }
                break;
            case Sub: {
                auto B = state.pop();
                state.top() -= B;
            }
                break;
            case Mul: {
                auto B = state.pop();
                state.top() *= B;
            }
                break;
            case Div: {
                auto B = state.pop();
                state.top() /= B;
            }
                break;
            case Pow: {
                auto B = state.pop();
                state.top() = glm::pow(state.top(), glm::min(vec4(0), B));
            }
                break;
            case Noise: {
                auto detail = state.pop().x;
                auto scale = state.top().x;
                auto n = std::clamp(perlin.octaveNoise0_1(scale * sp.texCoord.x, scale * sp.texCoord.y, detail),
                        0.0,
                                    1.0);
                state.top() = vec4(n);
            }
                break;
            case Image: {
                RGBAImage *image;
                std::memcpy(&image, &inst.operand, sizeof(RGBAImage *));
                MIYUKI_CHECK(image != nullptr);
                auto color = image ? (*image)(mod(vec2(sp.texCoord.x, 1.0f - sp.texCoord.y), vec2(1))) : Spectrum(0);
                state.push(vec4(color, 1.0));
            }
                break;
            case Mix: {
                auto frac = state.pop()[0];
                auto B = state.pop();
                auto A = state.top();
                state.top() = lerp(A, B, vec4(frac));
            }
                break;
            case ColorRamp: {
                auto frac = state.pop().x;
                auto right = state.pop();
                auto left = state.pop();
                auto maxVal = state.pop().x;
                auto minVal = state.pop().x;
                state.push(lerp(left, right, vec4(std::clamp((frac - minVal) / (maxVal - minVal), 0.0f, 1.0f))));
            }
                break;
        }
        state.pc++;
    }
    MIYUKI_CHECK(state.sp == 1);
    return Spectrum(state.top());
}

void miyuki::core::ExprShader::preprocess() {
    Shader::preprocess();
    using namespace shading;
    engine = std::make_unique<ExecutionEngine>();
    images.clear();
    auto compileFunc = [&](const json &e, auto &compile) -> void {
        if (e.is_number()) {
            engine->addInstruction(Instruction{Push, vec4(e.get<float>())});
        } else if (e.is_array()) {
            auto op = e.at(0).get<std::string>();
            if (op == "float") {
                engine->addInstruction(Instruction{Push, vec4(e.at(1).get<float>())});
            } else if (op == "float3") {
                engine->addInstruction(Instruction{Push, vec4(e.at(1).get<vec3>(), 1)});
            } else if (op == "rgb") {
                if (e.at(1).is_array()) {
                    engine->addInstruction(Instruction{Push, vec4(e.at(1).get<vec3>(), 1)});
                } else if (e.at(1).is_string()) {
                    std::istringstream in(e.at(1).get<std::string>());
                    size_t rgb;
                    in >> std::hex >> rgb;
                    auto b = rgb & 0xffUL;
                    auto g = (rgb & 0xff00UL) >> 8UL;
                    auto r = (rgb & 0xff0000UL) >> 16UL;
                    engine->addInstruction(Instruction{Push, vec4(vec3(r, g, b) / 255.0f, 1)});
                } else {
                    MIYUKI_THROW(std::runtime_error, "Unexpected RGB format");
                }
            } else if (op == "add" || op == "+") {
                for (int i = 1; i < e.size(); i++) {
                    compile(e.at(i), compile);
                    if (i > 1) {
                        engine->addInstruction(Instruction{Add, {}});
                    }
                }
            } else if (op == "sub" || op == "-") {
                for (int i = 1; i < e.size(); i++) {
                    compile(e.at(i), compile);
                    if (i > 1) {
                        engine->addInstruction(Instruction{Sub, {}});
                    }
                }
            } else if (op == "mul" || op == "*") {
                for (int i = 1; i < e.size(); i++) {
                    compile(e.at(i), compile);
                    if (i > 1) {
                        engine->addInstruction(Instruction{Mul, {}});
                    }
                }
            } else if (op == "div" || op == "/") {
                for (int i = 1; i < e.size(); i++) {
                    compile(e.at(i), compile);
                    if (i > 1) {
                        engine->addInstruction(Instruction{Div, {}});
                    }
                }
            } else if (op == "pow") {
                compile(e.at(1), compile);
                compile(e.at(2), compile);
                engine->addInstruction(Instruction{Pow, {}});
            } else if (op == "noise") {
                compile(e.at(1), compile);
                compile(e.at(2), compile);
                engine->addInstruction(Instruction{Noise, {}});
            } else if (op == "color-ramp") {
                compile(e.at(1), compile); // minVal
                compile(e.at(2), compile); // maxVal
                compile(e.at(3), compile); // left
                compile(e.at(4), compile); // right
                compile(e.at(5), compile); //frac
                engine->addInstruction(Instruction{ColorRamp, {}});
            } else if (op == "image") {
                auto image = ImageLoader::getInstance()->loadRGBAImage(fs::path(e.at(1).get<std::string>()));
                images.emplace_back(image);
                engine->addInstruction(Instruction::Image(image.get()));
            } else {
                MIYUKI_THROW(std::runtime_error, fmt::format("Unknown operator {} ", op));
            }
        } else {
            MIYUKI_THROW(std::runtime_error, "Expression cannot be object");
        }
    };
    try {
        compileFunc(expr, compileFunc);
    } catch (std::exception &e) {
        MIYUKI_THROW(std::runtime_error, fmt::format("Error {} when compiling{}", e.what(), expr.dump(2)));
    }
}
