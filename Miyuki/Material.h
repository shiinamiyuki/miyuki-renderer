#pragma once
#include "Miyuki.h"

namespace Miyuki {
	class PathTracer;
	struct RenderContext;
	enum class BxDFType {
		none = 0,
		diffuse = 1,
		specular = 1 << 2,
		glossy = 1 << 3,
		emission = 1 << 16,
		all = diffuse | specular | glossy
	};
	inline bool hasBxDFType(BxDFType a, BxDFType b) {
		return (unsigned int)a & (unsigned int)b;
	}
	struct Material
	{
		vec3 emittance;
		vec3 diffuse;
		vec3 specular;
		double roughness;
		double Tr;
		double Ni;
		Material() :roughness(0),Tr(0),Ni(1) {}
		Material(const vec3&Ka, const vec3&Kd, const vec3&Ks)
			:emittance(Ka), diffuse(Kd), specular(Ks) {}
		bool render(PathTracer*,RenderContext &)const; // true for continue rendering, false for termination
		BxDFType sample(Seed * Xi, const vec3& wi, const vec3& norm, vec3& wo,vec3& rad, Float &)const;
		Float brdf(const vec3& wi, const vec3& norm, const vec3& wo)const;
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
	vec3 refract(Seed* Xi,const vec3&dir, const vec3 &norm, const Float Ni, Float & prob);
}
