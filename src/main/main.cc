#include <miyuki.h>
#include <reflection.h>

class A : public Miyuki::Reflection::Object {
public:
	MYK_CLASS(A, Miyuki::Reflection::Object)
	MYK_PROPERTY_BEGIN() {

	}MYK_PROPERTY_END()
};
class B : public A {
public:
	MYK_CLASS(B, A)
	MYK_PROPERTY_BEGIN() {

	}MYK_PROPERTY_END()
};

int main(int argc, char** argv) {
	using Miyuki::Reflection::Object;
	A *a = new A();
	fmt::print("{}\n", a->typeName());
	A* a2 = new B();
	Object* obj1 = a;
	Object* obj2 = a2;
	fmt::print("{} {}\n", obj1->typeName(), obj2->typeName());
	Object* a3 = obj1->getClass().create();
	fmt::print("{}\n", a3->typeName());
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	/*Miyuki::IO::ObjLoadInfo info(nullptr);
	Miyuki::IO::LoadObjFile(R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)", info);
	std::cout << info.mtlDescription.dump(2) << std::endl;*/
}