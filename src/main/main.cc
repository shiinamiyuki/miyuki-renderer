#include <miyuki.h>
#include <ui/mainwindow.h>
#include <graph/graph.h>

namespace Miyuki {
	namespace Graph {
		class TestNode : public Node {
			MYK_NODE_MEMBER(int, a)
			MYK_NODE_MEMBER(Float, b)
			MYK_NODE_MEMBER(Float, c)
		public:
			TestNode() :Node("TestNode") {
				init_a();
				init_b();
				init_c();
			}
			const char* type()const {
				return "TestNode";
			}
		};
	}
}


int main(int argc, char** argv) {
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	using namespace Miyuki;
	Graph::TestNode node;
	json j;
	node.serialize(j);
	std::cout << j.dump(2) << std::endl;
}