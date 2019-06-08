#ifndef MIYUKI_MATERIAL_NODE_H
#define MIYUKI_MATERIAL_NODE_H

#include <graph/graphcompiler.h>
#include <reflection.h>


namespace Miyuki {
	namespace Graph {
		class GraphCompiler;
		class Material;
		class MaterialNode : public Reflection::Object {
		public:
			MYK_CLASS(MaterialNode, Reflection::Object);
			virtual Material* createMaterial(GraphCompiler&)const { throw NotImplemented(); }
		};

		class MixedMaterialNode : public Reflection::Object {
		public:
			MYK_CLASS(MaterialNode, Reflection::Object);
			virtual Material* createMaterial(GraphCompiler&)const;
		private:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(MaterialNode, matA);
			MYK_PROPERTY(MaterialNode, matB);
			MYK_END_PROPERTY;
		};
	}
}

#endif 