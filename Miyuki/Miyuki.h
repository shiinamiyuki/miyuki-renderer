#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <set>
#include <array>
#include <map>
#include <queue>
#include "lib/rand48/erand48.h"
#include "vec.h"
#include "lib/lodepng/lodepng.h"
#include "lib/tiny_obj_loader/tiny_obj_loader.h"
//#include "lib/cxxopts.hpp"
#include "lib/fmt/format.h"

#define USE_PPL

#ifdef USE_PPL
#include <ppl.h>
#include <concurrent_vector.h>
#endif
namespace Miyuki {
	using Color = vec3;
	using Seed = unsigned short;
	const Float eps = 0.01;
	constexpr Float pi = 3.1415926535897;
	constexpr Float inf = 1000000000;
	template<typename T>
	using vector = std::vector<T>;
	template<typename F>
	inline void For(unsigned int a, unsigned int b, F f) {
		for (int i = a; i < b; i++) { f(i); }
	}
	template<typename F>
	inline void parallelFor(unsigned int a, unsigned int b, F f) {
#ifdef _MSC_VER
		concurrency::parallel_for(a, b, f);
#else
#omp parallel for
	for (int i = a; i < b; i++) { f(i); }
#endif
	}

	inline void parallelInvoke(std::function<void(void)>f1, std::function<void(void)>f2) {
		concurrency::parallel_invoke(f1, f2);
	}
	inline Float max(const vec3 & v) {
		return std::max(std::max(v.x(), v.y()), v.z());
	}
	inline vec3 randomVectorInHemisphere(const vec3 &norm, Seed *Xi) {
		double r1 = 2 * pi * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
		const auto &w = norm;
		auto u = vec3::crossProduct((abs(w.x()) > 0.1) ? vec3{ 0, 1, 0 } : vec3{ 1, 0, 0 }, w);
		u.normalize();
		auto v = vec3::crossProduct(w, u);
		auto r = vec3(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
		r.normalize();
		return r;
	}
	inline vec3 randomVectorInSphere(Seed *Xi) {
		const Float u1 = erand48(Xi);
		const Float u2 = erand48(Xi);
		const Float zz = 1.0 - 2.0 * u1;
		const Float r = sqrt(std::max(0.0, 1.0 - zz * zz));
		const Float phi = 2.0 * pi * u2;
		const Float xx = r * cos(phi);
		const Float yy = r * sin(phi);
		return vec3(xx, yy, zz);
	}
}

#define ENABLE_SIMD