#pragma once

#include <core/probablity/pdf.hpp>

#include <math/sampling.h>
namespace Miyuki {
	namespace Core {

		struct CosineHemispherePDF : PDF<CosineHemispherePDF> {
			Vec3f sampleImpl(const Point2f& u) const {
				return CosineWeightedHemisphereSampling(u);
			}
			Float evaluateImpl(const Vec3f& p)const {
				return std::abs(p.z) * INVPI;
			}
		};
		struct UniformspherePDF : PDF<UniformspherePDF> {
			Vec3f sampleImpl(const Point2f& u) const {
				return UniformSampleSphere(u);
			}
			Float evaluateImpl(const Vec3f& p) const {
				return UniformSpherePdf();
			}
		};
	}
}