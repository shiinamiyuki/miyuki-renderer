//
// Created by Shiina Miyuki on 2019/3/20.
//

#ifndef MIYUKI_GUI_H
#define MIYUKI_GUI_H

#include <miyuki.h>

namespace Miyuki {
    class GenericGUIWindow {
    public:
        GenericGUIWindow(int argc, char **argv) {}

        virtual ~GenericGUIWindow() {}

        virtual void update() = 0;

        virtual void render() = 0;

        virtual void show() = 0;
    };
}
#endif //MIYUKI_GUI_H
