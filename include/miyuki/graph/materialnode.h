#ifndef MIYUKI_MATERIAL_NODE_H
#define MIYUKI_MATERIAL_NODE_H

#include <graph/graphcompiler.h>
#include <graph/graphnode.h>
#include <graph/shadernode.h>

namespace Miyuki {
	namespace Graph {
		class GraphCompiler;
		class Material;
		class MaterialNode : public GraphNode {
		public:
			MYK_CLASS(MaterialNode, GraphNode);
			virtual Material* createMaterial(GraphCompiler&)const { throw NotImplemented(); }
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(std::string, name);
			MYK_PROPERTY(ShaderNode, emission);
			MYK_END_PROPERTY;
			const std::string getName()const {
				if (name) {
					return name->getValue();
				}
				return "";
			}
		};

		class MixedMaterialNode : public MaterialNode {
		public:
			MYK_FINAL_CLASS(MixedMaterialNode, MaterialNode);
			//virtual Material* createMaterial(GraphCompiler&)const;
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Float, fraction);
			MYK_PROPERTY(MaterialNode, matA);
			MYK_PROPERTY(MaterialNode, matB);
			MYK_END_PROPERTY;
		};

		class DiffuseMaterialNode : public MaterialNode {
		public:
			MYK_CLASS(DiffuseMaterialNode, MaterialNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(ShaderNode, roughness);
			MYK_PROPERTY(ShaderNode, color);
			MYK_END_PROPERTY;
		};

		class GlossyMaterialNode : public MaterialNode {
		public:
			MYK_CLASS(GlossyMaterialNode, MaterialNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(ShaderNode, roughness);
			MYK_PROPERTY(ShaderNode, color);
			MYK_END_PROPERTY;
		};
	}
}

#endif 