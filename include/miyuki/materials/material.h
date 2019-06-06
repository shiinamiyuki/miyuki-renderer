#ifndef MIYUKI_MATERIAL_H
#define MIYUKI_MATERIAL_H
#include <miyuki.h>
#include <graph/graph.h>


namespace Miyuki {
	enum BSDFLobe {
		kNone = 0,
		kDiffuse = 1 << 0,
		kGlossy = 1 << 2,
		kSpecular = 1 << 3,
		kReflection = 1 << 4,
		kTransmission = 1 << 5,
		kAll = kDiffuse | kSpecular | kGlossy | kReflection | kTransmission
	};

	class Material : public Graph::Node{
		BSDFLobe _lobe;
	public:
		Material(const std::string &name,  BSDFLobe lobe, Graph::Graph* G)
			:Graph::Node(name, G), _lobe(lobe) {}
		BSDFLobe lobe()const { return _lobe; }
	};

	class MixedMaterial : public Material {
		Float fraction;
	public:
		MixedMaterial(const std::string& name, Graph::Graph* G)
			:Material(name, G) {
		}
	};
}

#endif