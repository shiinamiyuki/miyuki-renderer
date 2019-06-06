#ifndef MIYUKI_EDGE_HPP
#define MIYUKI_EDGE_HPP
#include <miyuki.h>
namespace Miyuki {
	namespace Graph {
		class Node;
		struct Edge {
			Node* from;
			Node* to;
			std::string name;
			Edge(Node* from, Node* to, const std::string& name)
				:from(from), to(to), name(name) {}
			virtual const Edge& get()const = 0;
			virtual Edge& get() = 0;
		};

		template<typename T = Node>
		struct EdgeT : Edge {				
			EdgeT(Node* from, T* to, const std::string& name)
				:Edge(from, to, name) {}
			T* operator->() {
				return to;
			}
			T& operator *() {
				return *to;
			}
			const T& operator * () const{
				return *to;
			}
			virtual const Edge& get()const { return *this; }
			virtual Edge& get() { return *this; }
		};
	}
}

#endif