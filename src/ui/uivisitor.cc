#include <ui/uivisitor.h>
#include <ui/mykui.h>
#include <graph/graph.h>
#include <graph/materialnode.h>
#include <graph/meshnode.h>

namespace Miyuki {
	namespace GUI {
		class TypeSelector {
			std::unordered_map<std::string, const Reflection::Class*>_map;
		public:
			TypeSelector& select(const std::string& s, const Reflection::Class* c) {
				_map[s] = c;
				return *this;
			}
		};
		void UIVisitor::init() {
			visit<Graph::Graph>([=](Graph::Graph* node)->void {
				TreeNode().name("Materials").with(true, [=]() {
					auto materials = node->materials;
					int selected = -1;
					for (int i = 0; i < materials->size();i++) {
						auto material = materials->at(i);
						if (!material) {
							continue;
						}
						if (material->selected() && selected < 0) {
							selected = i;
						}
						const auto& name = material->getName();
						SingleSelectableText().name(name).selected(material->selected())
							.with(true, [=, &selected]() {
							selected = i;
						}).show();
					}
					for (int i = 0; i < materials->size(); i++) {
						auto material = materials->at(i);
						if (i == selected) {
							material->select();
						}
						else {
							material->unselect();
						}
					}
				}).show();
			});
		}
	}
}