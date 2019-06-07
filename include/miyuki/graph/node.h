#ifndef MIYUKI_NODE_H
#define MIYUKI_NODE_H
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
#include <graph/edge.hpp>

namespace Miyuki {
	namespace Graph {
		class Graph;
		class Node;

		class INodeVisitor {
		public:
			virtual void visit(Node*) = 0;
		};		

		class Node : NonCopyMovable {
			friend class Graph;
			Graph* _graph = nullptr;
			std::string _name;
			bool _expanded = false;
			bool _selected = false;
		public:
			bool selected()const { return _selected; }
			bool expaneded()const { return _expanded; }
			void select() { _selected = true; }
			void unselect() { _selected = false; }
			void expaned() { _expanded = true; }
			void collapse() { _expanded = false; }
			Node(const std::string& name, Graph* graph = nullptr) :_graph(graph), _name(name) {}
			Graph* graph()const { return _graph; }

			virtual const std::vector<Edge*> subnodes()const = 0;

			const Edge& byKey(const std::string& key)const {
				return (*std::find_if(subnodes().begin(), subnodes().end(),
					[&](const Edge* edge) {return edge->get().name == key; }))->get();
			}

			Edge& byKey(const std::string& key) {
				return (*std::find_if(subnodes().begin(), subnodes().end(),
					[&](const Edge* edge) {return edge->get().name == key; }))->get();
			}

			virtual void serialize(json& j)const;
			virtual const char* type()const = 0;
			virtual bool isLeaf()const { return false; }
			const std::string& name()const { return _name; }
		};

		class IDeserializer {
		public:
			virtual Node* deserialize(const json&, Graph*) = 0;
		};
	}
}
#endif