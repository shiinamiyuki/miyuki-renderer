#ifndef MIYUKI_BSDF_SCATTERINGFUNCTION_HPP
#define MIYUKI_BSDF_SCATTERINGFUNCTION_HPP

#include <core/probablity/commonpdf.hpp>

namespace Miyuki {
	namespace Core {	

		inline bool SameHemisphere(const Vec3f& w1, const Vec3f& w2) {
			return w1.z() * w2.z() >= 0;
		}

		inline Float CosTheta(const Vec3f& w) {
			return w.z();
		}

		inline Float AbsCosTheta(const Vec3f& w) {
			return fabs(CosTheta(w));
		}

		inline Float Cos2Theta(const Vec3f& w) {
			return w.z() * w.z();
		}

		inline Float Sin2Theta(const Vec3f& w) {
			return std::max(Float(0), 1 - Cos2Theta(w));
		}

		inline Float SinTheta(const Vec3f& w) {
			return std::sqrt(Sin2Theta(w));
		}

		inline Float TanTheta(const Vec3f& w) {
			return SinTheta(w) / CosTheta(w);
		}

		inline Float Tan2Theta(const Vec3f& w) {
			return Sin2Theta(w) / Cos2Theta(w);
		}

		inline Float CosPhi(const Vec3f& w) {
			auto s = SinTheta(w);
			return s == 0 ? 1.0f : clamp(w.x() / s, -1.0f, 1.0f);
		}

		inline Float SinPhi(const Vec3f& w) {
			auto s = SinTheta(w);
			return s == 0 ? 0.0f : clamp(w.y() / s, -1.0f, 1.0f);
		}

		inline Float Cos2Phi(const Vec3f& w) {
			auto c = CosPhi(w);
			return c * c;
		}

		inline Float Sin2Phi(const Vec3f& w) {
			auto s = SinPhi(w);
			return s * s;
		}

		inline Float CosDPhi(const Vec3f& wa, const Vec3f& wb) {
			return clamp<Float>((wa.x() * wb.x() + wa.y() * wb.y()) /
				std::sqrt((wa.x() * wa.x() + wa.y() * wa.y()) *
				(wb.x() * wb.x() + wb.y() * wb.y())), -1.0f, 1.0f);
		}

		inline Vec3f Reflect(const Vec3f& wo, const Vec3f& n) {
			return -1 * wo + 2 * Vec3f::dot(wo, n) * n;
		}

		inline Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) {
			cosThetaI = clamp(cosThetaI, -1, 1);
			bool entering = cosThetaI > 0.f;
			if (!entering) {
				std::swap(etaI, etaT);
				cosThetaI = std::abs(cosThetaI);
			}
			Float sinThetaI = std::sqrt(std::max((Float)0,
				1 - cosThetaI * cosThetaI));
			Float sinThetaT = etaI / etaT * sinThetaI;
			if (sinThetaT >= 1)
				return 1;
			Float cosThetaT = std::sqrt(std::max((Float)0,
				1 - sinThetaT * sinThetaT));

			Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
				((etaT * cosThetaI) + (etaI * cosThetaT));
			Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
				((etaI * cosThetaI) + (etaT * cosThetaT));
			return (Rparl * Rparl + Rperp * Rperp) / 2;
		}


		struct ScatteringFunctionSample {
			Point2f u;
			Vec3f wo;
			Vec3f wi; // sampled direction
			Float pdf; // pdf of the sampled direction
			Spectrum f; // sampled bsdf
		};

		/*
		*	A single scattering function
		*/
		template<class ImplT>
		struct ScatteringFunction {
			void sample(
				ScatteringFunctionSample& sample)const {
				This().sampleImpl(sample);
			}

			Spectrum evaluate(const Vec3f& wo, const Vec3f& wi)const {
				return This().evaluateImpl(wo, wi, lobe)
			}

			// evaluate pdf according to wo, wi
			Float evaluatePdf(const Vec3f& wo, const Vec3f& wi)const {
				return This().evaluatePdfImpl(wo, wi);
			}
		private:
			const ImplT& This()const {
				return *static_cast<ImplT*>(this);
			}
		};
	}
}


#endif