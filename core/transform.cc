//
// Created by Shiina Miyuki on 2019/1/19.
//

#include "transform.h"

using namespace Miyuki;

Transform::Transform() : translation(), rotation(),scale(1) {

}

Transform::Transform(const Vec3f &t, const Vec3f &r,Float s) : translation(t), rotation(r),scale(s) {

}

Vec3f Transform::apply(const Vec3f &_v)const {
    auto v = _v * scale;
    v = rotate(v, Vec3f(1, 0, 0), rotation.y());
    v = rotate(v, Vec3f(0, 1, 0), rotation.x());
    v = rotate(v, Vec3f(0, 0, 1), rotation.z());
    return v + translation;
}
