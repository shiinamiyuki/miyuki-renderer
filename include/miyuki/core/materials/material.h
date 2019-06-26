#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <math/transform.h>
#include <core/intersection.hpp>
namespace Miyuki {
	enum BSDFLobe {
		kNone = 0,
		kDiffuse = 1 << 0,
		kGlossy = 1 << 2,
		kSpecular = 1 << 3,
		kReflection = 1 << 4,
		kTransmission = 1 << 5,
		kAll = kDiffuse | kSpecular | kGlossy | kReflection | kTransmission,
		kAllButSpecular = kAll & ~kSpecular
	};

	class Material  {
		BSDFLobe lobe()const { return _lobe; }
		Material(BSDFLobe lobe) :_lobe(lobe) {}
	protected:
		BSDFLobe _lobe = kNone;
	};

	class MixedMaterial : public Material {
		Material* matA, * matB;
	public:
	};

	// Some user specific sample option
	enum BSDFSampleOption {
		kNoSampleOption = 0,
		kUseStrictNormal = 1,
	};

	class Sampler;
	// Conventions:
	// sampled directions w.r.t local frame
	// sampled direction is always incoming direction (wi)
	// pdf w.r.t solid angle
	struct BSDFSample {
		Float uPick; // picking bsdf
		Point2f uSample; //sampling bsdf
		Vec3f wo;
		Vec3f wi; // sampled direction
		Float pdf; // pdf of the sampled direction
		Spectrum f; // sampled bsdf
		BSDFLobe lobe; //sampled lobe
		BSDFSampleOption option;
		BSDFSample(BSDFSampleOption opt) :option(opt), pdf(0) {}
	/*	BSDFSample(Intersection& isct, Sampler* sampler, BSDFSampleOption opt)
			:option(opt), uSample(sampler->get2D()), uPick(sampler->get1D()) {
			wo = isct.world_to_local(isct.wo);
		}*/
	};

	class BSDF {
		BSDFLobe lobe;
		CoordinateSystem localFrame;
		Vec3f Ng;
	public:
		void sample(
			BSDFSample& sample
		)const;

		// evaluate bsdf according to wo, wi
		Spectrum evaluate(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe = BSDFLobe::kAll
		)const;

		// evaluate pdf according to wo, wi
		Float evaluatePdf(
			const Vec3f& wo,
			const Vec3f& wi,
			BSDFSampleOption option,
			BSDFLobe lobe = BSDFLobe::kAll
		)const;

		Vec3f localToWorld(const Vec3f& w) const {
			return localFrame.localToWorld(w);
		}

		Vec3f worldToLocal(const Vec3f& w) const {
			return localFrame.worldToLocal(w);
		}
	};
}

#endif