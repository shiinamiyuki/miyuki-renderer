#include <ui/guinodevisitor.h>
#include <ui/input.h>
#include <ui/editable.h>
#include <imgui/imgui.h>
#include <utils/log.h>

namespace Miyuki {
	namespace GUI {
		struct Helper : Graph::INodeVisitor {
			std::vector<std::shared_ptr<EditEntryBase>> entries;
			std::set<Graph::Node*> visited;
			template<typename T>
			void visitT(const std::string& prompt, T* node) {
				auto temp = node->getValue();
				auto opt = GetInput(prompt, temp);
				if (opt.has_value()) {
					//
					Log::log("value changed\n");
				}
			}
			void visit(const std::string& prompt, Graph::IntNode* node) {
				visitT(prompt, node);
			}

			void visit(const std::string& prompt, Graph::Float3Node* node) {
				visitT(prompt, node);
			}

			void visit(const std::string& prompt, Graph::FloatNode* node) {
				visitT(prompt, node);
			}

			void visit(const std::string& prompt, Graph::TranformNode* node) {
				visitT(prompt, node);
			}

			void visit(Graph::Edge& edge) {
				auto node = edge.to;
				// never visit the same node twice
				if (visited.find(node) != visited.end()) {
					return;
				}
				visited.insert(node);
				if (node->isLeaf()) {
					auto leaf = static_cast<Graph::BasicLeafNode*>(node);
					switch (leaf->leafType()) {
					case Graph::kInt: {
						visit(node->name(), static_cast<Graph::IntNode*>(leaf));
						break;
					}
					case Graph::kFloat: {
						visit(node->name(), static_cast<Graph::FloatNode*>(leaf));
						break;
					}
					case Graph::kFloat3: {
						visit(node->name(), static_cast<Graph::Float3Node*>(leaf));
						break;
					}
					case Graph::kTransform: {
						visit(node->name(), static_cast<Graph::TranformNode*>(leaf));
						break;
					}
					default:
						throw NotImplemented();
					}
				}
				else {
					if (ImGui::TreeNode(node->name().c_str())) {
						for (auto& i : node->subnodes()) {
							visit(i);
						}
						ImGui::TreePop();
					}
				}
			}
			void visit(Graph::Node* root) {
				Graph::Edge dummy(nullptr, root, "");
				visit(dummy); 
			}
		};

		void GuiNodeVisitor::visit(Graph::Node* root) {
			Helper h;
			h.visit(root);
		}

		void GuiNodeVisitor::show(Graph::Node* root) {
			visit(root);
		}
	}
}