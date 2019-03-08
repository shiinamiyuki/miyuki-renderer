//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_OBJ2JSON_H
#define MIYUKI_OBJ2JSON_H

#include "miyuki.h"
#include "utils/jsonparser.hpp"

namespace Miyuki {
    std::tuple<Json::JsonObject, Json::JsonObject> MTL2Json(const std::string &);
    void MergeMTL2Json(const std::string& mtl, const std::string & json);
}
#endif //MIYUKI_OBJ2JSON_H
