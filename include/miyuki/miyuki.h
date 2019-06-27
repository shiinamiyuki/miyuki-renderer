//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_MIYUKI_H
#define MIYUKI_MIYUKI_H


#pragma warning(disable:4244)
#pragma warning(disable:4018)
#define NOMINMAX
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <thread>
#include <chrono>
#include <ctime>
#include <xmmintrin.h>
#include <immintrin.h>
#include <algorithm>
#include <functional>
#include <random>
#include <limits>
#include <mutex>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <deque>
#include <condition_variable>
#include <optional>
#include <iostream>
#include <filesystem>
#include <future>
#include <stack>


#include <tiny_obj_loader/tiny_obj_loader.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <lodepng/lodepng.h>
#include <hilbert/hilbert_curve.hpp>



using nlohmann::json;
namespace Miyuki {
	using Float = float;

	class NotImplemented : public std::logic_error {
	public:
		NotImplemented() : std::logic_error("Function not yet implemented") {};
	};

	inline void __Assert(bool expr) {
		if (!expr) {
			throw std::runtime_error(std::string("Assertion failed"));
		}
	}

	template<typename T>
	T clamp(T x, T a, T b) {
		return std::min(std::max(a, x), b);
	}

	namespace cxx = std;
#define CHECK(expr) do{if(!(expr)){fmt::print(stderr, "{}:{} {} failed\n",__FILE__, __LINE__, #expr);}}while(0)
#define Assert(expr) do{if(!(expr)){fmt::print(stderr, "{}:{} {} failed\n",__FILE__, __LINE__, #expr);__Assert(false);}}while(0)
	void Init();

	void Exit();
	const Float PI = 3.1415926535f;
	const Float PI2 = PI / 2.0f;
	const Float PI4 = PI / 4.0f;
	const Float INVPI = 1.0f / PI;
	const Float INV4PI = 1.0f / (4 * PI);
	const Float INV2PI = 1.0f / (2 * PI);
}

#include <math/vec.hpp>
#include <math/spectrum.h>

#endif //MIYUKI_MIYUKI_H