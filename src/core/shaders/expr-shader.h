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

#ifndef MIYUKIRENDERER_EXPR_SHADER_H
#define MIYUKIRENDERER_EXPR_SHADER_H

#include <miyuki.foundation/defs.h>
#include <miyuki.renderer/shader.h>
#include <miyuki.foundation/image.hpp>

namespace miyuki::core {
    namespace shading {
        // A op B -> push A; push B; op;
        enum Opcode : uint8_t {
            Push,
            Add,
            Sub,
            Mul,
            Div,
            Pow,
            Mix, // A, B, frac
            ColorRamp, // minVal, maxVal, left, right, frac
            Noise, // scale detail
            Image  // pointer
        };

        struct Instruction {
            Opcode opcode;
            Vec3f operand;

            static Instruction Image(RGBAImage *p) {
                Vec3f data;
                RGBAImage *tmp = p;
                std::memcpy(&data, &tmp, sizeof(RGBAImage *));
                return Instruction{Opcode::Image, data};
            }
        };

        struct ExecutionState {
            int pc = 0;
            int sp = 0;
            static const size_t stackSize = 64;
            std::array<Vec3f, stackSize> stack{};

            auto &top() { return stack[sp - 1]; }

            auto pop() {
                auto v = top();
                sp--;
                return v;
            }

            void push(Vec3f v){
                stack[sp++] = v;
            }
        };

        class ExecutionEngine {
            std::vector<Instruction> code;
        public:
            void addInstruction(const Instruction &instruction) { code.emplace_back(instruction); }

            Spectrum execute(const ShadingPoint &sp);
        };
    }
    class ExprShader : public Shader {
        json expr;
        std::unique_ptr<shading::ExecutionEngine> engine;
        std::vector<std::shared_ptr<RGBAImage>> images;
    public:
        MYK_DECL_CLASS(ExprShader, "ExprShader", interface = "Shader")

        MYK_SER(expr)

        [[nodiscard]] Spectrum evaluate(const ShadingPoint &point) const override {
            return engine->execute(point);
        }

        void preprocess()override;
    };
}
#endif //MIYUKIRENDERER_EXPR_SHADER_H
