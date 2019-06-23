#include <graph/graphnode.h>
#include <graph/visitor.h>
namespace Miyuki {
	namespace Graph {
		void GraphNode::accept(Visitor& visitor) {
			visitor.visit(this);
		}
	}
}