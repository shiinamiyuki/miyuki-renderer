//
// Created by Shiina Miyuki on 2019/2/6.
//

#include "materialfactory.h"
using namespace Miyuki;

MaterialPtr MaterialMaker::operator()(const MaterialInfo &info) {
    return Miyuki::MaterialPtr();
}
