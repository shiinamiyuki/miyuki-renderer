#pragma once

#include <reflection.h>

namespace Miyuki {
	namespace Graph {
		class GraphNode : public Reflection::Object {
			bool _expanded = false;
			bool _selected = false;
		public: 
			MYK_CLASS(GraphNode, Reflection::Object);
			bool selected()const { return _selected; }
			bool expanded()const { return _expanded; }
			void select() { _selected = true; }
			void unselect() { _selected = false; }
			void expand() { _expanded = true; }
			void collpase() { _expanded = false; }
		};


		template<class T>
		class UnionNode : public Reflection::Union<T, GraphNode> {
			using __Base = Reflection::Union<T, GraphNode>;
		public:
			MYK_CLASS(UnionNode<T>, __Base);
			MYK_BEGIN_PROPERTY;
			MYK_END_PROPERTY;
		};

		class TransformNode : public Reflection::Object {
		public:
			MYK_CLASS(TransformNode, Reflection::Object);
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Reflection::FloatNode, scale);
			MYK_PROPERTY(Reflection::Float3Node, translation);
			MYK_PROPERTY(Reflection::Float3Node, rotation);
			MYK_END_PROPERTY;
			void init() {
				translation = allocator->New< Reflection::Float3Node>();
				rotation = allocator->New< Reflection::Float3Node>();
				scale = allocator->New< Reflection::FloatNode>(1.0f);
			}
		};
	}
}