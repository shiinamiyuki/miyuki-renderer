#pragma once
#include "Miyuki.h"

namespace Miyuki {
	class PathTracer;
	struct RenderContext;
	struct Material
	{
		vec3 emittance;
		vec3 diffuse;
		vec3 specular;
		vec3 ambient;
		double roughness;
		double Tr;
		double Ni;
		Material() :roughness(0),Tr(0),Ni(1) {}
		Material(const vec3&Ka, const vec3&Kd, const vec3&Ks)
			:emittance(Ka), diffuse(Kd), specular(Ks) {}
		bool render(PathTracer*,RenderContext &)const; // true for continue rendering, false for termination
		bool sample(const vec3& wi, const vec3& norm, vec3& wo, Float &);
		static Material makeRefr(const vec3&spec, double Ni) {
			Material m(vec3(0, 0, 0), vec3(0, 0, 0), spec);
			m.Tr = 1;
			m.Ni = Ni;
			return m;
		}
		static Material makeEmission(const vec3&c) {
			Material m(c,vec3(0, 0, 0), vec3(0, 0, 0));
			return m;
		}
		static Material makeDiffuse(const vec3&c) {
			Material m(vec3(0, 0, 0),c, vec3(0, 0, 0));
			return m;
		}
		static Material makeSpec(const vec3&c) {
			Material m(vec3(0, 0, 0),  vec3(0, 0, 0), c);
			return m;
		}
		~Material() {}
	};
	inline vec3 reflect(const vec3& dir, const vec3 & norm) {
		return dir - 2 * vec3::dotProduct(dir, norm)*norm;
	}
	vec3 refract(const vec3&dir, const vec3 &norm, const Float Ni, Float & prob);
}
