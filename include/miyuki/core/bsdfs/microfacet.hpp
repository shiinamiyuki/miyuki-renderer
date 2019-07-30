#ifndef MIYUKI_BSDF_MICROFACET_HPP
#define MIYUKI_BSDF_MICROFACET_HPP

#include <core/probablity/commonpdf.hpp>
#include <core/bsdfs/trignometry.hpp>
#include <math/func.h>

namespace Miyuki {
	namespace Core {

		enum MicrofacetType {
			EBeckmann,
			EGGX
		};
		template<class Model>
		struct MicrofacetModel {
			Float D(const Vec3f& wh) const {
				return This().DImpl(wh);
			}
			Float lambda(const Vec3f& w) const {
				return This().lambdaImpl(w);
			}
			Float G1(const Vec3f& w) const {
				return 1 / (1 + lambda(w));
			}
			Float G(const Vec3f& wo, const Vec3f& wi) const {
				return 1 / (1 + lambda(wo) + lambda(wi));
			}
			Vec3f sampleWh(const Vec3f& wo,
				const Point2f& u) const {
				return This().sampleWhImpl(wo, u);
			}
			Float evaluatePdf(const Vec3f& wh)const {
				return This().evaluatePdfImpl(wh);
			}
		private:
			const Model& This()const {
				return *static_cast<const Model*>(this);
			}
		};

		struct BeckmannDistribution : MicrofacetModel<BeckmannDistribution> {
			Float lambdaImpl(const Vec3f& w)const {
				Float absTanTheta = std::abs(TanTheta(w));
				if (std::isinf(absTanTheta))return 0.0f;
				Float a = 1.0f / (alpha * absTanTheta);
				if (a >= 1.6f)
					return 0;
				return (1 - 1.259f * a + 0.396f * a * a) /
					(3.535f * a + 2.181f * a * a);
			}
			Float DImpl(const Vec3f& wh)const {
				Float a2 = alpha * alpha;
				Float tan2Theta = Tan2Theta(wh);
				if (std::isinf(tan2Theta))return 0.0f;
				Float cos2Theta = Cos2Theta(wh);
				Float cos4Theta = cos2Theta * cos2Theta;
				return std::exp(-tan2Theta / a2)
					/ (PI * a2 * cos4Theta);
			}
			BeckmannDistribution(Float alpha) :alpha(alpha) {}

			Vec3f sampleWhImpl(const Vec3f& wo, const Point2f& u)const {
				Float logSample = std::log(1 - u[0]);
				if (std::isinf(logSample))logSample = 0;
				Float tan2Theta = -alpha * alpha * logSample;
				Float phi = 2 * u[1] * PI;
				Float cosTheta = 1.0f / std::sqrt(1.0f + tan2Theta);
				Float sinTheta = std::sqrt(std::max(0.0f, 1 - cosTheta * cosTheta));
				return SphericalToXYZ(sinTheta, cosTheta, phi);
			}
			Float evaluatePdfImpl(const Vec3f& wh)const {
				return D(wh) * AbsCosTheta(wh);
			}
		private:
			Float alpha;
		};

		struct MicrofacetWrapper : MicrofacetModel<MicrofacetWrapper> {
			MicrofacetType model;
			union {
				BeckmannDistribution beckmann;
			};
			MicrofacetWrapper(MicrofacetType model, Float alpha):model(model) {
				switch (model) {
				case EBeckmann:
					beckmann = BeckmannDistribution(alpha);
					break;
				}
			}
			Float lambdaImpl(const Vec3f& w)const {
				switch (model) {
				case EBeckmann:
					return beckmann.lambdaImpl(w);
				}
				CHECK(false);
				return 0.0f;
			}
			Float DImpl(const Vec3f& wh)const {
				switch (model) {
				case EBeckmann:
					return beckmann.DImpl(wh);
				}
				CHECK(false);
				return 0.0f;
			}

			Vec3f sampleWhImpl(const Vec3f& wo, const Point2f& u)const {
				switch (model) {
				case EBeckmann:
					return beckmann.sampleWhImpl(wo, u);
				}
				CHECK(false);
				return {};
			}
			Float evaluatePdfImpl(const Vec3f& wh)const {
				switch (model) {
				case EBeckmann:
					return beckmann.evaluatePdfImpl(wh);
				}
				CHECK(false);
				return 0.0f;
			}
		};
	}
}


#endif