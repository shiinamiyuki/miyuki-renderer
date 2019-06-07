#ifndef MIYUKI_EDGE_HPP
#define MIYUKI_EDGE_HPP
#include <miyuki.h>
namespace Miyuki {
	namespace Graph {
		class Node;
		struct Edge {
			Node* from = nullptr;
			Node* to = nullptr;
			std::string name;
			Edge( Node* from, Node* to, const std::string& name)
				:from(from), to(to), name(name) {}
			Edge(Node * from, const std::string& name):from(from), name(name){}
			virtual const Edge& get()const = 0;
			virtual Edge& get() = 0;
		};
		
		// Type erasure
		template<typename T = Node>
		struct EdgeT : Edge {	
			static_assert(std::is_base_of<Node, T>::value);
			EdgeT(Node* from, T* to, const std::string& name)
				:Edge(from, to, name) {}
			EdgeT(Node* from, const std::string& name) :Edge(from, name) {}
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