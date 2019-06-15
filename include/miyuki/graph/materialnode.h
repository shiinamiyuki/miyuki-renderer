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
		};

		class MixedMaterialNode : public MaterialNode {
		public:
			MYK_CLASS(MixedMaterialNode, MaterialNode);
			virtual Material* createMaterial(GraphCompiler&)const;
		private:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Float, fraction);
			MYK_PROPERTY(MaterialNode, matA);
			MYK_PROPERTY(MaterialNode, matB);
			MYK_END_PROPERTY;
		};

		class DiffuseMaterialNode : public MaterialNode {
		public:
			MYK_CLASS(DiffuseMaterialNode, MaterialNode);
		private:
			MYK_BEGIN_PROPERTY;
		private:
			MYK_PROPERTY(ShaderNode, roughness);
			MYK_PROPERTY(ShaderNode, color);
			MYK_END_PROPERTY;
		};

		class GlossyMaterialNode : public MaterialNode {
		public:
			MYK_CLASS(GlossyMaterialNode, MaterialNode);
		private:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(ShaderNode, roughness);
			MYK_PROPERTY(ShaderNode, color);
			MYK_END_PROPERTY;
		};
	}
}

#endif 