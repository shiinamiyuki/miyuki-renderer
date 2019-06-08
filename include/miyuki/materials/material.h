#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <reflection.h>


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

	class Material : public Reflection::Object {
	public:
		MYK_CLASS(Material, Reflection::Object);
		BSDFLobe lobe()const { return _lobe; }
	protected:
		BSDFLobe _lobe = kNone;
	};

	class MixedMaterial : public Material {
	public:
		MYK_CLASS(MixedMaterial, Material);
		MYK_BEGIN_PROPERTY;
	private:
		MYK_PROPERTY(Material, matA);
		MYK_PROPERTY(Material, matB);
		MYK_END_PROPERTY;
	};
	
}

#endif