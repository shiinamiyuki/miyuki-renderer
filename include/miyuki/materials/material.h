#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>

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
	
}

#endif