#ifndef MIYUKI_GRAPHDETAIL_H
#define MIYUKI_GRAPHDETAIL_H

#include <miyuki.h>
#include <utils/noncopymovable.hpp>

namespace Miyuki {
	namespace Graph {
		class Graph;
		class Node;

		class INodeVisitor {
		public:
			virtual void visit(Node*) = 0;
		};

		struct Edge {
			Node* from, * to;
			std::string name;
			Edge(Node* from, Node* to, const std::string& name)
				:from(from), to(to), name(name) {}
		};

		class Node : NonCopyMovable {
			friend class Graph;
			Graph* _graph = nullptr;
			std::vector<Edge> _subnodes;
			std::string _name;
		public:
			Node(const std::string& name, Graph* graph = nullptr) :_graph(graph), _name(name) {}
			Graph* graph()const { return _graph; }
			Edge makeEdge(const std::string& name, Node* to) {
				return Edge(this, to, name);
			}

			const std::vector<Edge>& subnodes()const { return _subnodes; }
			std::vector<Edge>& subnodes() { return _subnodes; }

			void set(const std::string& name, Node* node = nullptr) {
				auto it = std::find_if(_subnodes.begin(), _subnodes.end(),
					[&](const Edge & edge) {return edge.name == name; });
				if (it == _subnodes.end()) {
					_subnodes.emplace_back(this, node, name);
				}
			}

			const Edge& byKey(const std::string& name)const {
				return *std::find_if(_subnodes.begin(), _subnodes.end(),
					[&](const Edge & edge) {return edge.name == name; });
			}

			Edge& byKey(const std::string& name) {
				return *std::find_if(_subnodes.begin(), _subnodes.end(),
					[&](const Edge & edge) {return edge.name == name; });
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

		class Graph {
			std::random_device rd;
			std::uniform_int_distribution<uint64_t> dist;
			std::vector<Node*> _nodeList;

			// owns the nodes
			std::unordered_map<std::string, std::unique_ptr<Node>> _allNodes;

			std::unordered_map<std::string, std::unique_ptr<IDeserializer>> _deserializers;
			Graph();
			void deserialize(const json&);
		public:
			std::string generateUniqueName(const std::set<std::string>&);
			std::string generateUniqueName();
			bool addNode(const std::string&, std::unique_ptr<Node>);
			void serialize(json&);
			static std::unique_ptr<Graph> CreateGraph(const json&);
			void registerDeserializer(const std::string&, std::unique_ptr<IDeserializer>);
		};
	}
}


#endif