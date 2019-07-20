#include <ui/mainwindow.h>


void printInterfaceGraph(const std::string& I, int indent = 0) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	auto& types = detail::Types::get();
	auto& i = types._tree.at(I);
	auto printIndent = [](int indent) {
		for (int k = 0; k < indent; k++) {
			fmt::print("  ");
		}
	};
	printIndent(indent);
	fmt::print("{}:\n", i->name);
	for (auto& impl : i->directImpl) {
		printIndent(indent + 1);
		fmt::print("{}\n", impl->name());
	}
	for (auto& d : i->derived) {
		printInterfaceGraph(d->name, indent + 1);
	}
}

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	/*GUI::MainWindow window(argc, argv);
	window.show();*/
	try {
		printInterfaceGraph(Component::interfaceInfo());
	}
	catch (std::out_of_range& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
