#include <miyuki.h>
#include <ui/mainwindow.h>
#include <graph/leaf.h>
using namespace Miyuki;
int main(int argc, char** argv) {
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	
	Graph::Float3Node node("a", Vec3f(1,2,3));
	json j;
	node.serialize(j);
	std::stringstream stream;
	stream << j.dump(2) << std::endl;
	std::string s = stream.str();
	std::cout << s << std::endl;
	json o = {};
	o["a"] = j;
	auto graph = Graph::Graph::CreateGraph(o);
	j = {};
	graph->serialize(j);
	std::cout << j << std::endl;
}