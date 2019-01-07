#ifndef BXDF_H
#define BXDF_H
#include "trace.h"

#define BxDF_None			(0)
#define BxDF_Lambertian		(1)
#define BxDF_Specular		(1<<1)
#define BxDF_Refractive		(1<<2)
#define BxDF_Glossy			(1<<3)

inline bool isDelta(float pdf) {
	return pdf > inf;
}
inline float pdf(__global const Material * m,const Vector wo, const Vector norm, int bxdfType) {
	float p = 0;
	if (bxdfType & BxDF_Lambertian) {
		p += dot(wo, norm) / pi;	// cosine weighted hemisphere sampling
	}
	if (bxdfType & BxDF_Specular) {
		p += inf;
	}
	if (bxdfType & BxDF_Refractive) {
		p += inf;
	}
	return p;
}
// https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
inline float GGXDistribution(const Vector m, const Vector n, float alpha_g) {
	alpha_g *= alpha_g;
	float d = dot(m, n);
	if (d <= 0)return 0;
	return max(0.0, alpha_g / (pi  * pow(d * d* (alpha_g - 1) + 1, 2)) + 0.001);
}

#endif