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

#include "../src/core/integrators/sdtree.hpp"

namespace miyuki::core {


    void testDTree() {
        auto image = ImageLoader::getInstance()->loadRGBAImage(
                fs::path("../data/living_room/textures/picture11-vert.jpg"));
        // image->write("test.png", 1.0f);
        DTree dTree;
        double area = (image->dimension[0] * image->dimension[1]);
        for (int iter = 0; iter < 4; iter++) {
            for (int i = 0; i < image->dimension[0]; i++) {
                for (int j = 0; j < image->dimension[1]; j++) {
                    //log::log("{} {}\n",i,j);
                    auto p = Point2f(i, j) / Point2f(image->dimension);
                    auto rgba = (*image)(p) / (float) area;
                    core::Spectrum color(rgba.x(), rgba.y(), rgba.z());
                    dTree.deposit(p, color.luminance());
                }
            }


            DTree copy = dTree;
            dTree.refine(copy, 0.01);
//            result.write(fmt::format("out{}.png", iter + 1), 1.0f / 2.2f);
        }

        RGBAImage result(image->dimension);
        ParallelFor(0, image->dimension[0], [&](int i, int) {
            for (int j = 0; j < image->dimension[1]; j++) {
                auto p = Point2f(i, j) / Point2f(image->dimension);
                result(i, j) = dTree.pdf(p);
            }
        });
        result.write("out.png", 1.0f / 2.2f);
        double sum = 0;
        double cnt = 0;
        core::Rng rng;
        for (int i = 0; i < 100000; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
            auto p = dTree.sample(u);
            auto pdf = dTree.pdf(p);
            sum += 1.0f / pdf;
            cnt += 1.0;
        }
        log::log("{}\n", sum / cnt);

    }

    void testDTree2() {
        DTreeWrapper dTree;
        core::Rng rng;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 100000; j++) {
                dTree.deposit(normalize(Vec3f(rng.uniformFloat(), rng.uniformFloat(), rng.uniformFloat())), 1.0f);
            }
            dTree.refine();
        }
        double sum = 0;
        double cnt = 0;
        for (int i = 0; i < 100000; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
            auto w = dTree.sample(u);
            sum += 1.0 / dTree.pdf(w);
            cnt += 1;
        }
        log::log("{}\n", sum / cnt);
    }

    void testDTree3() {
        DTree dTree;
        core::Rng rng;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 100000; j++) {
                auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
                u = powN<7>(u);
                dTree.deposit(u, 1.0f);
            }
            auto copy = dTree;
            dTree.refine(copy, 0.01);
        }
        double sum = 0;
        double cnt = 0;
        for (int i = 0; i < 100000; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());

            auto w = dTree.sample(u);
            //log::log("{}\n",dTree.eval(w)/dTree.pdf(w));
            sum += 1.0 / dTree.pdf(w);
            cnt += 1;
        }
        log::log("{}\n", sum / cnt);
    }

    void testDTree4() {
        DTree dTree;
        core::Rng rng;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 100000; j++) {
                auto w = normalize(Vec3f(rng.uniformFloat(), rng.uniformFloat(), rng.uniformFloat()) * 2.0f - 1.0f);
                auto u = dirToCanonical(w);
//                log::log("{} {}\n",u[0],u[1]);
//                u = powN<7>(u);
                dTree.deposit(
                        u,
                        1.0f);
            }
            auto copy = dTree;
            dTree.refine(copy, 0.01);
        }
        double sum = 0;
        double cnt = 0;
        for (int i = 0; i < 100; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
            auto w = dTree.sample(u);
//            log::log("{}\n",dTree.eval(w)/dTree.pdf(w));
//            log::log("{} {}\n",w[0],w[1]);
            sum += 1.0 / dTree.pdf(w);
            cnt += 1;
        }
        log::log("{}\n", sum / cnt);
    }

    void testSDTree() {
        STree sTree(Bounds3f{Point3f(0), Point3f(1)});
        core::Rng rng;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 100000; j++) {
                sTree.deposit(Point3f(rng.uniformFloat(), rng.uniformFloat(), rng.uniformFloat()),
                              normalize(
                                      Vec3f(rng.uniformFloat(), rng.uniformFloat(), rng.uniformFloat()) * 2.0f - 1.0f),
                              1.0f);
            }
            sTree.refine(100);
        }
        double sum = 0;
        double cnt = 0;
        for (int i = 0; i < 100000; i++) {
            auto u = Point2f(rng.uniformFloat(), rng.uniformFloat());
            auto p = float3(0.3);
            auto w = sTree.sample(p, u);
            sum += 1.0 / sTree.pdf(p, w);
            cnt += 1;
        }
        log::log("{}\n", sum / cnt);

        auto v = normalize(Vec3f{-0.3, -0.7,-0.2});
        log::log("{} {} {}\n", v.x(), v.y(), v.z());
        v = canonicalToDir(dirToCanonical(v));
        log::log("{} {} {}\n", v.x(), v.y(), v.z());
    }
}


int main() {
//    miyuki::core::testDTree();
//    miyuki::core::testDTree2();
//    miyuki::core::testDTree3();
    miyuki::core::testDTree4();
    miyuki::core::testSDTree();
}