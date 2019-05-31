#include <ui/guinodevisitor.h>
#include <ui/input.h>


namespace Miyuki {
	namespace GUI {
		struct Helper : Graph::INodeVisitor {
			std::set<Graph::Node*> visited;

			void visit(Graph::IntNode * node) {
				
			}

			void visit(Graph::Float3Node* node) {

			}

			void visit(Graph::FloatNode* node) {

			}

			void visit(Graph::TranformNode* node) {

			}

			void visit(Graph::Node* node) {
				// never visit the same node twice
				if (visited.find(node) != visited.end()) {
					return;
				}
				visited.insert(node);
				if (node->isLeaf()) {
					auto leaf = static_cast<Graph::BasicLeafNode*>(node);
					switch (leaf->leafType) {
					case Graph::kInt: {
						visit(static_cast<Graph::IntNode*>(leaf));
						break;
					}
					case Graph::kFloat: {
						visit(static_cast<Graph::FloatNode*>(leaf));
						break;
					}
					case Graph::kFloat3: {
						visit(static_cast<Graph::Float3Node*>(leaf));
						break;
					}
					case Graph::kTransform: {
						visit(static_cast<Graph::TranformNode*>(leaf));
						break;
					}
					default:
						throw NotImplemented();
					}
				}
			}
		};
		void GuiNodeVisitor::visit(Graph::Node*) {
		}

		void GuiNodeVisitor::show(Graph::Node* root) {
			visit(root);
		}
	}
}