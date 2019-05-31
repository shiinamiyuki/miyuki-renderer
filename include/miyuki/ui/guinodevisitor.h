#ifndef MIYUKI_GUINODEVISITOR_H
#define MIYUKI_GUINODEVISITOR_H

#include <graph/graph.h>


namespace Miyuki {
	namespace GUI {
		class GuiNodeVisitor : public Graph::INodeVisitor {
			void visit(Graph::Node*)override;
		public:
			void show(Graph::Node* root);
		};
	}
}

#endif