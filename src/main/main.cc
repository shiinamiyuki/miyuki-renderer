#include <miyuki.h>
#include <ui/mainwindow.h>
#include <graph/graph.h>
#include <io/importobj.h>

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
	Miyuki::GUI::MainWindow window(argc, argv);
	window.show();
	/*Miyuki::IO::ObjLoadInfo info(nullptr);
	Miyuki::IO::LoadObjFile(R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)", info);
	std::cout << info.mtlDescription.dump(2) << std::endl;*/
}