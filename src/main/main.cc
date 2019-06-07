#include <miyuki.h>
#include <reflection.h>

class A : public Miyuki::Reflection::Object {
public:
	MYK_CLASS(A, Miyuki::Reflection::Object);
	MYK_BEGIN_PROPERTY;
	MYK_PROPERTY(Miyuki::Reflection::Object, a);
	MYK_PROPERTY(int, b);
	MYK_END_PROPERTY; 
};
class B : public A {
public:
	MYK_CLASS(B, A);
	MYK_BEGIN_PROPERTY;
	MYK_PROPERTY(Miyuki::Reflection::Array<int>, arr);
	MYK_PROPERTY(Miyuki::Reflection::Object, a2);
	MYK_PROPERTY(A, a3);
	MYK_PROPERTY(int, b2);
	MYK_END_PROPERTY;
};
struct T {
	int a;
	T* b;
};
int main(int argc, char** argv) {
	using Miyuki::Reflection::Object;
	A* a = new A();	
	fmt::print("{}\n", a->typeName());
	B* a2 = new B();
	a2->a = new Miyuki::Reflection::IntNode(1);
	a2->a3 = a;
	Object* obj1 = a;
	Object* obj2 = a2;
	fmt::print("{} {}\n", obj1->typeName(), obj2->typeName());
	Object* a3 = obj1->getClass().create();
	fmt::print("{}\n", a3->typeName());
	fmt::print("{}\n", a3->baseName());
	for (const auto& i : a2->getProperties()) {
		fmt::print("{}\n", i->name);
	}
	json j;
	a2->arr = new Miyuki::Reflection::Array<int>();
	a2->arr->push_back(new Miyuki::Reflection::IntNode(1));
	fmt::print("{}\n", a2->arr.object->typeName());
	a2->serialize(j);
	std::cout << j.dump(2) << std::endl;
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	/*Miyuki::IO::ObjLoadInfo info(nullptr);
	Miyuki::IO::LoadObjFile(R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)", info);
	std::cout << info.mtlDescription.dump(2) << std::endl;*/
}