#ifndef MIYUKI_KERNEL_MATH_FUNC_H
#define MIYUKI_KERNEL_MATH_FUNC_H

#include "kerneldef.h"


MYK_KERNEL_NS_BEGIN

MYK_KERNEL_FUNC_INLINE
float3 reflect(float3 wo, float3 n) {
	return -1.0f * wo + 2 * dot(wo, n) * n;
}

#pragma region trignomety
MYK_KERNEL_FUNC_INLINE
bool is_same_hemisphere(const float3& w1, const float3 w2) {
	return w1.z * w2.z >= 0;
}

MYK_KERNEL_FUNC_INLINE
float cos_theta(const float3 w) {
	return w.z;
}

MYK_KERNEL_FUNC_INLINE
float abs_cos_theta(const float3 w) {
	return fabs(cos_theta(w));
}

MYK_KERNEL_FUNC_INLINE
float cos2_theta(const float3 w) {
	return w.z * w.z;
}

MYK_KERNEL_FUNC_INLINE
float sin2_theta(const float3 w) {
	return max(Float(0), 1 - cos2_theta(w));
}

MYK_KERNEL_FUNC_INLINE
float sin_theta(const float3 w) {
	return sqrt(sin2_theta(w));
}

MYK_KERNEL_FUNC_INLINE
float tan_theta(const float3 w) {
	return sin_theta(w) / cos_theta(w);
}

MYK_KERNEL_FUNC_INLINE
float tan2_theta(const float3 w) {
	return sin2_theta(w) / cos2_theta(w);
}

MYK_KERNEL_FUNC_INLINE
float cos_phi(const float3 w) {
	auto s = sin_theta(w);
	return s == 0 ? 1.0f : clamp(w.x / s, -1.0f, 1.0f);
}

MYK_KERNEL_FUNC_INLINE
float sin_phi(const float3 w) {
	auto s = sin_theta(w);
	return s == 0 ? 0.0f : clamp(w.y / s, -1.0f, 1.0f);
}

MYK_KERNEL_FUNC_INLINE
float cos2_phi(const float3 w) {
	auto c = cos_phi(w);
	return c * c;
}

MYK_KERNEL_FUNC_INLINE
float sin2_phi(const float3 w) {
	auto s = sin_phi(w);
	return s * s;
}

MYK_KERNEL_FUNC_INLINE
float cosd_phi(const float3 wa, const float3 wb) {
	return clamp((wa.x * wb.x + wa.y * wb.y) /
		sqrt((wa.x * wa.x + wa.y * wa.y) *
		(wb.x * wb.x + wb.y * wb.y)), -1.0f, 1.0f);
}
#pragma endregion bsdf trignomety function

MYK_KERNEL_FUNC_INLINE
float fr_dielectric(float cosThetaI, float etaI, float etaT) {
	cosThetaI = clamp(cosThetaI, -1, 1);
	bool entering = cosThetaI > 0.f;
	if (!entering) {
		float tmp = etaI;
		etaI = etaT;
		etaT = tmp;
		cosThetaI = abs(cosThetaI);
	}
	float sinThetaI = sqrt(max((float)0,
		1 - cosThetaI * cosThetaI));
	float sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1)
		return 1;
	float cosThetaT = sqrt(max((float)0,
		1 - sinThetaT * sinThetaT));

	float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
		((etaT * cosThetaI) + (etaI * cosThetaT));
	float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
		((etaI * cosThetaI) + (etaT * cosThetaT));
	return (Rparl * Rparl + Rperp * Rperp) / 2;
}

MYK_KERNEL_FUNC_INLINE
float3 cosine_hemisphere_sampling(float2 u) {
	float theta = u.x * 2 * PI;
	float r = sqrt(u.y);
	return make_float3(sin(theta) * r, cos(theta) * r, sqrt(1 - r * r));
}

MYK_KERNEL_FUNC_INLINE
float cosine_hemiphsere_pdf(float3 v) {
	return fabs(v.z) * INVPI;
}



MYK_KERNEL_FUNC_INLINE
 float3 spherical_to_xyz(Float sinTheta, Float cosTheta, Float phi) {
	return make_float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

MYK_KERNEL_FUNC_INLINE
Float spherical_theta(const float3 v) {
	return acos(clamp(v.z, -1.0f, 1.0f));
}

MYK_KERNEL_FUNC_INLINE
Float spherical_phi(const float3 v) {
	auto p = atan2(v.y, v.x);
	return p < 0 ? (p + 2 * PI) : p;
}

 
MYK_KERNEL_FUNC_INLINE Float power_heuristics(Float pdf1, Float pdf2) {
	Float p1 = pdf1 * pdf1;
	Float p2 = pdf2 * pdf2;
	return p1 / (p1 + p2);
}

#pragma region error function
// From pbrt
MYK_KERNEL_FUNC_INLINE 
Float erfinv(Float x) {
	Float w, p;
	x = clamp(x, -.99999f, .99999f);
	w = -std::log((1 - x) * (1 + x));
	if (w < 5) {
		w = w - 2.5f;
		p = 2.81022636e-08f;
		p = 3.43273939e-07f + p * w;
		p = -3.5233877e-06f + p * w;
		p = -4.39150654e-06f + p * w;
		p = 0.00021858087f + p * w;
		p = -0.00125372503f + p * w;
		p = -0.00417768164f + p * w;
		p = 0.246640727f + p * w;
		p = 1.50140941f + p * w;
	}
	else {
		w = std::sqrt(w) - 3;
		p = -0.000200214257f;
		p = 0.000100950558f + p * w;
		p = 0.00134934322f + p * w;
		p = -0.00367342844f + p * w;
		p = 0.00573950773f + p * w;
		p = -0.0076224613f + p * w;
		p = 0.00943887047f + p * w;
		p = 1.00167406f + p * w;
		p = 2.83297682f + p * w;
	}
	return p * x;
}

// From pbrt
MYK_KERNEL_FUNC_INLINE 
Float fasterf(Float x) {
	// constants
	Float a1 = 0.254829592f;
	Float a2 = -0.284496736f;
	Float a3 = 1.421413741f;
	Float a4 = -1.453152027f;
	Float a5 = 1.061405429f;
	Float p = 0.3275911f;

	// Save the sign of x
	int sign = 1;
	if (x < 0) sign = -1;
	x = abs(x);

	// A&S formula 7.1.26
	Float t = 1 / (1 + p * x);
	Float y =
		1 -
		(((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

	return sign * y;
}
#pragma endregion

#pragma region microfacet

MYK_KERNEL_FUNC_INLINE
float beckmann_lambda(const float3 w, Float alpha) {
	Float absTanTheta = abs(tan_theta(w));
	if (isinf(absTanTheta))return 0.0f;
	Float a = 1.0f / (alpha * absTanTheta);
	if (a >= 1.6f)
		return 0;
	return (1 - 1.259f * a + 0.396f * a * a) /
		(3.535f * a + 2.181f * a * a);
}

MYK_KERNEL_FUNC_INLINE
Float beckmann_d(const float3 wh, Float alpha) {
	Float a2 = alpha * alpha;
	Float tan2Theta = tan2_theta(wh);
	if (isinf(tan2Theta))return 0.0f;
	Float cos2Theta = cos2_theta(wh);
	Float cos4Theta = cos2Theta * cos2Theta;
	return exp(-tan2Theta / a2)
		/ (PI * a2 * cos4Theta);
}

MYK_KERNEL_FUNC_INLINE
float3 beckmann_sample_wh(const float3 wo, const float2 u, float alpha) {
	Float logSample = log(1 - u.x);
	if (isinf(logSample))logSample = 0;
	Float tan2Theta = -alpha * alpha * logSample;
	Float phi = 2 * u.y * PI;
	Float cosTheta = 1.0f / sqrt(1.0f + tan2Theta);
	Float sinTheta = sqrt(max(0.0f, 1 - cosTheta * cosTheta));
	return spherical_to_xyz(sinTheta, cosTheta, phi);
}

#pragma endregion 

MYK_KERNEL_NS_END
#endif