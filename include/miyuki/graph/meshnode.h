#pragma once

#include <graph/graphcompiler.h>
#include <graph/graphnode.h>
namespace Miyuki {
	namespace Graph {
		class ObjectNode : public GraphNode {
		public:
			MYK_CLASS(ObjectNode, GraphNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::StringNode, objectName);
			MYK_PROPERTY(Reflection::StringNode, materialName);
			MYK_END_PROPERTY;
		};

		class MeshNode: public GraphNode {
		public:
			MYK_CLASS(MeshNode, GraphNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::FileNode, meshFile);
			MYK_PROPERTY(TransformNode, transform);
			MYK_PROPERTY(Reflection::Array<ObjectNode>, objects);
			MYK_PROPERTY(Reflection::StringNode, name);
			MYK_END_PROPERTY;
			MYK_INIT() {
				objects = allocator->New< Reflection::Array<ObjectNode>>();
			}
		};

		
	}
}