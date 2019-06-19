#include <ui/mainwindow.h>
#include <reflection.h>
#include <graph/materialnode.h>
#include <io/importobj.h>

void test() {
	using namespace Miyuki;
	Reflection::Runtime runtime;
	json j;
	{
		auto arr = runtime.New< Reflection::Array<Reflection::IntNode>>();
		arr->push_back(runtime.New<Reflection::IntNode>(1));
		arr->push_back(runtime.New<Reflection::IntNode>(2));
		arr->push_back(runtime.New<Reflection::IntNode>(3));
		
		arr->staticCast<Reflection::Object>()->serialize(j);
	}
	runtime.collect();
	std::cout << j.dump(2) << std::endl;
	auto arr = runtime.deserialize(j);
	if (arr) {
		j = {};
		arr.value()->serialize(j);
		std::cout << j.dump(2) << std::endl;
	}
}

int main(int argc, char** argv) {
	test();
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	return 0;
}