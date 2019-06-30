#pragma once

#include <graph/graphcompiler.h>
#include <graph/graphnode.h>
#include <graph/materialnode.h>
namespace Miyuki {
	namespace Graph {
		class ObjectNode : public GraphNode {
		public:
			MYK_CLASS(ObjectNode, GraphNode);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::StringNode, objectName);
			MYK_PROPERTY(MaterialNode, material);
			MYK_END_PROPERTY;
			std::string getName() {
				return objectName->getValue();
			}
			void setName(const std::string& s) {
				objectName->setValue(s);
			}
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