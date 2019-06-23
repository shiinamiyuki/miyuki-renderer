#pragma once
#include <miyuki.h>
#include <reflection.h>
namespace Miyuki {
	namespace Graph {
		class GraphNode;

		class Visitor {
			using VisitFunc = std::function<void(GraphNode*)>;
			std::unordered_map<const Reflection::Class*, VisitFunc> funcs;
		protected:
			template<class T>
			void visit(std::function<void(T*)> func) {
				funcs[T::__classinfo__()] = [=](GraphNode* node) {
					func(Reflection::StaticCast<T>(node));
				};
			}
		public:
			void visit(GraphNode* node);
		};
	}
}