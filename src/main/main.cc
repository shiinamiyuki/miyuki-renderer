#include <miyuki.h>
#include <ui/mainwindow.h>
#include <graph/graph.h>
#include <io/importobj.h>
#include <materials/material.h>


int main(int argc, char** argv) {

	Miyuki::GUI::MainWindow window(argc, argv);
	window.show();
	/*Miyuki::IO::ObjLoadInfo info(nullptr);
	Miyuki::IO::LoadObjFile(R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)", info);
	std::cout << info.mtlDescription.dump(2) << std::endl;*/
}