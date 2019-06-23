#pragma once

#include <reflection.h>

namespace Miyuki {
	namespace Graph {
		class Visitor;
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
			void accept(Visitor&);
		};


		template<class T>
		class UnionNode : public Reflection::Union<T, GraphNode> {
			using __Base = Reflection::Union<T, GraphNode>;
		public:
			MYK_CLASS(UnionNode<T>, __Base);
			MYK_BEGIN_PROPERTY;
			MYK_END_PROPERTY;
		};
	}
}