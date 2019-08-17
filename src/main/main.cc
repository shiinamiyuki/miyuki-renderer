#include <ui/mainwindow.h>


namespace Miyuki {
	namespace Core {
		struct QuadTreeNode {
			uint32_t children[4];
		};
		struct QuadTree {

		};
	}
}
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	Init();
	GUI::MainWindow window(argc, argv);
	window.show();
	Exit();
	return 0;
}
