//
// Created by Shiina Miyuki on 2019/3/7.
//

#ifndef MIYUKI_FUNC_H
#define MIYUKI_FUNC_H

#include "miyuki.h"

namespace Miyuki {
    inline Float PowerHeuristics(Float pdf1, Float pdf2) {
        Float p1 = pdf1 * pdf1;
        Float p2 = pdf2 * pdf2;
        return p1 / (p1 + p2);
    }
}
#endif //MIYUKI_FUNC_H
