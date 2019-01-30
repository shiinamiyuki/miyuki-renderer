//
// Created by Shiina Miyuki on 2019/1/30.
//
#include "../core/transform.h"

#define  BOOST_TEST_MODULE TransformTest

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/filesystem/fstream.hpp>

namespace utf = boost::unit_test;
using namespace Miyuki;
BOOST_AUTO_TEST_CASE(TestIndentity, *utf::tolerance(float(0.0001))) {
    std::random_device rd;
    std::uniform_real_distribution<Float> dist(-1.0f, 1.f);
    auto I = Matrix4x4::identity();
    for (int i = 0; i < 1000; i++) {
        Vec3f v(dist(rd),dist(rd),dist(rd));
        Vec3f v2 = v;
        v = I.mult(v);
        BOOST_TEST(v.x() == v2.x());
        BOOST_TEST(v.y() == v2.y());
        BOOST_TEST(v.z() == v2.z());
    }
}
BOOST_AUTO_TEST_CASE(TestInverse, *utf::tolerance(float(0.001))) {
    std::random_device rd;
    std::uniform_real_distribution<Float> dist(-1.0f, 1.f);
    for (int i = 0; i < 1000; i++) {
        Matrix4x4 m;
        for(int k = 0;k<16;k++){
            m[k] = dist(rd);
        }
        Matrix4x4 inv;
        if(Matrix4x4::inverse(m, inv)){
            Vec3f v(dist(rd),dist(rd),dist(rd));
            Vec3f v2 = v;
            auto I = inv.mult(m);
            v = inv.mult(m.mult(v));
            BOOST_TEST(v.x() == v2.x());
            BOOST_TEST(v.y() == v2.y());
            BOOST_TEST(v.z() == v2.z());
        }
    }
}

BOOST_AUTO_TEST_CASE(TestTranslation, *utf::tolerance(float(0.0001))) {
    std::random_device rd;
    std::uniform_real_distribution<Float> dist(-10000.0f, 10000.0f);
    auto t = Transform();
    auto m = Matrix4x4::identity();
    for (int i = 0; i < 100; i++) {
        Vec3f tr(dist(rd), dist(rd), dist(rd));
        m = m.mult(Matrix4x4::translation(tr));
        t.translation += tr;
        Vec3f v(dist(rd), dist(rd), dist(rd));
        v.w() = 1;
        auto a = m.mult(v);
        auto b = t.apply(v);
        BOOST_TEST(a.x() == b.x());
        BOOST_TEST(a.y() == b.y());
        BOOST_TEST(a.z() == b.z());
    }
}