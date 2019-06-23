#include <graph/visitor.h>
#include <graph/graphnode.h>
namespace Miyuki {
	namespace Graph {
		void Visitor::visit(GraphNode* node) {
			funcs.at(&node->getClass())(node);
		}
	}
}