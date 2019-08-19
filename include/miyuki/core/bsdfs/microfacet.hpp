#ifndef MIYUKI_BSDF_MICROFACET_HPP
#define MIYUKI_BSDF_MICROFACET_HPP

#include <core/probablity/commonpdf.hpp>
#include <core/bsdfs/trignometry.hpp>
#include <core/bsdfs/bsdf.h>
#include <math/func.h>
#include <core/bsdfs/fresnel.hpp>
namespace Miyuki {
	namespace Core {

		enum MicrofacetType {
			EGGX,
			EBeckmann,
			EPhong,
			
		};

		inline Float BeckmannD(Float alpha, const Vec3f& m) {
			if (m.z <= 0.0f)return 0.0f;
			auto c = Cos2Theta(m);
			auto t = Tan2Theta(m);
			auto a2 = alpha * alpha;
			return std::exp(-t / a2) / (PI * a2 * c * c);
		}

		inline Float BeckmannG1(Float alpha, const Vec3f& v, const Vec3f& m) {
			if (Vec3f::dot(v, m) * v.z <= 0) {
				return 0.0f;
			}
			auto a = 1.0f / (alpha * TanTheta(v));
			if (a < 1.6) {
				return (3.535 * a + 2.181 * a * a) / (1.0f + 2.276 * a + 2.577 * a * a);
			}
			else {
				return 1.0f;
			}
		}
		inline Float PhongG1(Float alpha, const Vec3f& v, const Vec3f& m) {
			if (Vec3f::dot(v, m) * v.z <= 0) {
				return 0.0f;
			}
			auto a = std::sqrt(0.5f * alpha + 1.0f) / TanTheta(v);
			if (a < 1.6) {
				return (3.535 * a + 2.181 * a * a) / (1.0f + 2.276 * a + 2.577 * a * a);
			}
			else {
				return 1.0f;
			}
		}

		inline Float PhongD(Float alpha, const Vec3f& m) {
			if (m.z <= 0.0f)return 0.0f;
			return (alpha + 2) / (2 * PI) * std::pow(m.z, alpha);
		}

		inline Float GGX_D(Float alpha, const Vec3f& m) {
			if (m.z <= 0.0f)return 0.0f;
			Float a2 = alpha * alpha;
			auto c2 = Cos2Theta(m);
			auto t2 = Tan2Theta(m);
			auto at = (a2 + t2);
			return a2 / (PI * c2 * c2 * at * at);

		}

		inline Float GGX_G1(Float alpha, const Vec3f& v, const Vec3f& m) {
			if (Vec3f::dot(v, m) * v.z <= 0) {
				return 0.0f;
			}
			return 2.0 / (1.0 + std::sqrt(1.0 + alpha * alpha * Tan2Theta(m)));
		}
		// see https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
		struct MicrofacetModel {
			MicrofacetModel(MicrofacetType type, Float roughness) :type(type) {
				if (type == EPhong) {
					alpha = 2.0f / (roughness * roughness) - 2.0f;
				}
				else {
					alpha = roughness;
				}
			}
			Float D(const Vec3f& m)const {
				switch (type) {
				case EBeckmann:
					return BeckmannD(alpha, m);
				case EPhong:
					return PhongD(alpha, m);
				case EGGX:
					return GGX_D(alpha, m);
				}

				return 0.0f;
			}
			Float G1(const Vec3f& v, const Vec3f& m)const {
				switch (type) {
				case EBeckmann:
					return BeckmannG1(alpha, v, m);
				case EPhong:
					return PhongG1(alpha, v, m);
				case EGGX:
					return GGX_G1(alpha, v, m);
				}
				return 0.0f;
			}
			Float G(const Vec3f& i, const Vec3f& o, const Vec3f& m)const {
				return G1(i, m) * G1(o, m);
			}
			Vec3f sampleWh(const Vec3f& wo, const Point2f& u)const {
				Float phi = 2 * PI * u[1];
				Float cosTheta;
				switch (type) {
				case EBeckmann: {
					Float t2 = -alpha * alpha * std::log(1 - u[0]);
					cosTheta = 1.0f / std::sqrt(1 + t2);
					break;
				}
				case EPhong: {
					cosTheta = std::pow((double)u[0], 1.0 / ((double)alpha + 2.0f));
					break;
				}
				case EGGX: {
					Float t2 = alpha * alpha * u[0] / (1 - u[0]);
					cosTheta = 1.0f / std::sqrt(1 + t2);
					break;
				}
				}
				auto sinTheta = std::sqrt(std::max(0.0f, 1 - cosTheta * cosTheta));
				return Vec3f(std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta);
			}
			Float evaluatePdf(const Vec3f& wh)const {
				return D(wh) * AbsCosTheta(wh);
			}
		private:
			MicrofacetType type;
			Float alpha;
		};

		class GlossyBSDF : public BSDFComponent {
			const MicrofacetModel microfacet;
			const FresnelWrapper fresnel;
			const Vec3f R;
			const Float alpha;
		public:
			GlossyBSDF(Vec3f R, Float alpha, MicrofacetType type)
				:R(R), alpha(alpha), microfacet(type, alpha), fresnel(EPerfectSpecular),
				BSDFComponent(BSDFLobe(EGlossy | EReflection)) {

			}
			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				auto wh = microfacet.sampleWh(sample.wo, sample.u);
				sample.wi = Reflect(sample.wo, wh);
				ctx.assignWi(sample.wi);
				if (!SameHemisphere(sample.wo, sample.wi)) {
					sample.pdf = 0.0f;
					sample.f = {};
				}
				else {
					sample.pdf = microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(sample.wo, wh));
					sample.f = evaluate(ctx);

				}
				CHECK(!sample.f.hasNaNs());
				sample.lobe = getLobe();

			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				if (!SameHemisphere(wo, wi))return {};
				Float cosThetaO = AbsCosTheta(wo);
				Float cosThetaI = AbsCosTheta(wi);
				auto wh = (wo + wi);
				if (cosThetaI == 0 || cosThetaO == 0)return {};
				if (wh.x == 0 && wh.y == 0 && wh.z == 0)return {};
				wh.normalize();
				auto F = fresnel.evaluate(Vec3f::dot(wi, wh));
				return R * F * microfacet.D(wh) * microfacet.G(wo, wi, wh) * F / (4.0f * cosThetaI * cosThetaO);
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			) const override {
				auto& wo = ctx.wo();
				auto& wi = ctx.wi();
				if (!SameHemisphere(wo, wi))return 0.0f;
				auto wh = (wo + wi).normalized();
				return microfacet.evaluatePdf(wh) / (4.0f * Vec3f::dot(wo, wh));
			}
		};

		class SpecularBSDFImpl : public BSDFComponent {
			const Vec3f R;
		public:
			SpecularBSDFImpl(const Vec3f& R) :R(R), BSDFComponent(BSDFLobe(ESpecular | EReflection)) {}
			virtual void sample(
				BSDFEvaluationContext& ctx,
				BSDFSample& sample
			)const override {
				sample.wi = Reflect(sample.wo, Vec3f(0, 0, 1));
				sample.f = R / AbsCosTheta(sample.wi);
				sample.pdf = 1.0f;
				sample.lobe = getLobe();
				ctx.assignWi(sample.wi);
			}

			// evaluate bsdf according to wo, wi
			virtual Spectrum evaluate(
				const BSDFEvaluationContext& ctx
			)const override {
				return {};
			}

			// evaluate pdf according to wo, wi
			virtual Float evaluatePdf(
				const BSDFEvaluationContext& ctx
			) const override {
				return 0.0f;
			}

		};
	}
}


#endif