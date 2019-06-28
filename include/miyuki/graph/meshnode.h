#pragma once

#include <graph/graphcompiler.h>
#include <graph/graphnode.h>
namespace Miyuki {
	namespace Graph {
		class MeshNode: public GraphNode {
		public:
			MYK_CLASS(MeshNode, GraphNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::FileNode, meshFile);
			MYK_PROPERTY(TransformNode, transform);
			MYK_END_PROPERTY;
		};

		class ObjectNode : public GraphNode {
		public:
			MYK_CLASS(ObjectNode, GraphNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::StringNode, objectName);
			MYK_PROPERTY(Reflection::StringNode, materialName);
			MYK_END_PROPERTY;
		};
	}
}