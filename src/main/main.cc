#include <ui/mainwindow.h>
#include <reflection.h>
#include <graph/materialnode.h>
#include <io/importobj.h>

int main(int argc, char** argv) {
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	using namespace Miyuki;
	Reflection::Runtime runtime;
	IO::ObjLoadInfo info(&runtime);
	IO::LoadObjFile(
		R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)", info);
	std::ofstream out("out.json");
	out << info.mtlDescription.dump(2) << std::endl;
	return 0;
}