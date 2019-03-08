//
// Created by Shiina Miyuki on 2019/3/3.
//

#define BOOST_TEST_MODULE ParamaterTest

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/filesystem/fstream.hpp>

#include "core/parameter.h"
#include "utils/jsonparser.hpp"

BOOST_AUTO_TEST_SUITE(ParameterSetTestSuite)

    BOOST_AUTO_TEST_CASE(TestBasic) {
        using namespace Miyuki;
        ParameterSet set;
        set.addFloat("a",1.0f);
        set.addInt("b", 2);
        set.addVec3f("c", Vec3f(1,2,3));
        set.addString("d", "qwerty");
        BOOST_TEST(set.findFloat("a", 0.0f) == 1.0f);
        BOOST_TEST(set.findFloat("asa", 1.0f) == 1.0f);
        BOOST_TEST(set.findInt("b", 1.0f) == 2);
    }
    BOOST_AUTO_TEST_CASE(TestSerialization) {
        using namespace Miyuki;
        using namespace Miyuki::IO;
        ParameterSet set;
        set.addFloat("a",1.0f);
        set.addInt("b", 2);
        set.addVec3f("c", Vec3f(1,2,3));
        set.addString("d", "qwerty");
        auto obj = serialize(set);
        BOOST_TEST(obj["floats"]["a"].getFloat() == 1.0f);
        BOOST_TEST(obj["ints"]["b"].getInt() == 2);
        BOOST_TEST((deserialize<Vec3f>(obj["vec3fs"]["c"]) - Vec3f(1,2,3)).length() < 1e-3f);
        BOOST_TEST(obj["strings"]["d"].getString() == "qwerty");
        set = deserialize<ParameterSet>(obj);
        BOOST_TEST(set.findFloat("a", 0.0f) == 1.0f);
        BOOST_TEST(set.findInt("b", 1.0f) == 2);
    }

BOOST_AUTO_TEST_SUITE_END()