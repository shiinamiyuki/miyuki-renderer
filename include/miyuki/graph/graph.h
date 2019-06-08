#pragma once

#include <reflection.h>

namespace Miyuki {
	namespace Graph {
		class MaterialNode;
		class MeshNode;
		class IntegratorNode;
		class Graph : public Reflection::Object {
		public:
			MYK_CLASS(Graph, Reflection::Object);
		protected:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::Array<MaterialNode>, materials);
			MYK_PROPERTY(Reflection::Array<MeshNode>, meshes);
			MYK_PROPERTY(IntegratorNode, integrator);
			MYK_END_PROPERTY;
		};
	} 
}