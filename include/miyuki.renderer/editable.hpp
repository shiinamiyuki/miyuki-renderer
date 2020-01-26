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

#ifndef MIYUKIRENDERER_EDITABLE_HPP
#define MIYUKIRENDERER_EDITABLE_HPP

#include <miyuki.foundation/spectrum.h>
#include <miyuki.foundation/math.hpp>

namespace miyuki::core {
    class Editor;

    class Editable {
        bool _changed = true;
    public:
        virtual void updateOnChange() {}

        void resetChanged() { _changed = false; }

        virtual void edit(Editor &) {}

        void notify() {
            _changed = true;
        }
    };

    class Editor {
    public:
        virtual bool edit(const char *name, bool &value) {}

        virtual bool edit(const char *name, float &value) {}

        virtual bool edit(const char *name, int &value) {}

        virtual bool edit(const char *name, int2 &value) {}

        virtual bool edit(const char *name, int3 &value) {}

        virtual bool edit(const char *name, float2 &value) {}

        virtual bool edit(const char *name, float3 &value) {}

        virtual bool edit(const char *name, Spectrum &value) {}

        virtual bool edit(const char *name, std::string &value) {}

        virtual bool edit(const char *name, std::shared_ptr<serialize::Serializable> &ptr) {}
    };

    namespace editing {
        struct Visitor {
            Editable &editable;
            Editor &editor;

            Visitor(Editable &editable, Editor &editor) : editable(editable), editor(editor) {}
#define EDITING_GEN_VISIT_TY(ty) \
            void visit(ty &value, const char *name) {\
                if (editor.edit(name, value)) {\
                    editable.notify();\
                }\
            }

            EDITING_GEN_VISIT_TY(int)
            EDITING_GEN_VISIT_TY(float)
            EDITING_GEN_VISIT_TY(float2)
            EDITING_GEN_VISIT_TY(float3)
            EDITING_GEN_VISIT_TY(Spectrum)
            EDITING_GEN_VISIT_TY(std::string)
        };
    }

}

#endif //MIYUKIRENDERER_EDITABLE_HPP
