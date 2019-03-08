//
// Created by Shiina Miyuki on 2019/2/28.
//

#include "utils/obj2json.h"

using namespace Miyuki;

int main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    auto obj = argv[1];
    std::string out = argc > 2 ? argv[2] : "out.json";
    MergeMTL2Json(obj, out);
}
