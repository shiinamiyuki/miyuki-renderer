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
					for (auto material : *materials) {
						if (!material) {
							continue;
						}
						const auto& name = material->getName();
						Text().name(name).show();
						//
					}
				});
			});
		}
	}
}