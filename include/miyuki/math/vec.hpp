//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_VEC_HPP
#define MIYUKI_VEC_HPP

#include "miyuki.h"
#include "vec3.hpp"
namespace Miyuki {
	inline void to_json(json& j, const Vec3f& v) {
		j = json{
			v[0],v[1],v[2]
		};
	}

	inline void from_json(const json& j, Vec3f& v) {
		v[0] = j.at(0).get<Float>();
		v[1] = j.at(1).get<Float>();
		v[2] = j.at(2).get<Float>();
	}
	inline void to_json(json& j, const Point3f& v) {
		j = json{
			v[0],v[1],v[2]
		};
	}

	inline void from_json(const json& j, Point3f& v) {
		v[0] = j.at(0).get<Float>();
		v[1] = j.at(1).get<Float>();
		v[2] = j.at(2).get<Float>();
	}
	inline void to_json(json& j, const Point2f& v) {
		j = json{
			v[0],v[1]
		};
	}

	inline void from_json(const json& j, Point2f& v) {
		v[0] = j.at(0).get<Float>();
		v[1] = j.at(1).get<Float>();
	}
	inline void to_json(json& j, const Point2i& v) {
		j = json{
			v[0],v[1]
		};
	}

	inline void from_json(const json& j, Point2i& v) {
		v[0] = j.at(0).get<int>();
		v[1] = j.at(1).get<int>();
	}
}

#endif //MIYUKI_GEOMETRY_H
