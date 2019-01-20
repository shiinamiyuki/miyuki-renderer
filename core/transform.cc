//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "transform.h"

using namespace Miyuki;

Transform::Transform() : translation(), rotation() {

}

Transform::Transform(const Vec3f &t, const Vec3f &r) : translation(t), rotation(r) {

}

Vec3f Transform::apply(const Vec3f &) {
    return Vec3f();
}
