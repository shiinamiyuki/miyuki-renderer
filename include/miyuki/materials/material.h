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
		kAll = kDiffuse | kSpecular | kGlossy | kReflection | kTransmission,
		kAllButSpecular = kAll & ~kSpecular
	};

	class Material : public Graph::Node {
	protected:
		BSDFLobe _lobe;
	public:
		Material(const std::string& name, BSDFLobe lobe, Graph::Graph* G)
			:Graph::Node(name, G), _lobe(lobe) {}
		BSDFLobe lobe()const { return _lobe; }
	};

	class MixedMaterial : public Material {
		MYK_DECL_MEMBER(Float, fraction);
		MYK_DECL_MEMBER(Material, matA);
		MYK_DECL_MEMBER(Material, matB);
	public:
		MixedMaterial(const std::string& name, Graph::Graph* G)
			:Material(name, kNone, G) {
		}
		MYK_NODE_INFO_BEGIN() {
			MYK_NODE_MEMBER_INFO(fraction);
			MYK_NODE_MEMBER_INFO(matA);
			MYK_NODE_MEMBER_INFO(matB);
		}
		MYK_NODE_INFO_END()
	};
}

#endif