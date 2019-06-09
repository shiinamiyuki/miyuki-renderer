#include <materials/material.h>
#include <miyuki.h>
#include <reflection.h>
#include <utils/result.hpp>

class A : public Miyuki::Reflection::Object {
public:
	MYK_CLASS(A, Miyuki::Reflection::Object);
	MYK_BEGIN_PROPERTY;
	MYK_PROPERTY(Miyuki::Reflection::Object, a);
	MYK_PROPERTY(int, b);
	MYK_END_PROPERTY;
};
class B;
class B : public A {
public:
	MYK_CLASS(B, A);
	MYK_BEGIN_PROPERTY;
	MYK_PROPERTY(Miyuki::Reflection::Array<int>, arr);
	MYK_PROPERTY(Miyuki::Reflection::Object, a2);
	MYK_PROPERTY(A, a3);
	MYK_PROPERTY(B, a4);
	MYK_PROPERTY(int, b2);
	MYK_END_PROPERTY;
};

int main(int argc, char** argv) {
	using namespace Miyuki::Reflection;
	Runtime runtime;
	runtime.registerClass<IntNode>()
		.registerClass<FloatNode>()
		.registerClass<A>()
		.registerClass<B>()
		.registerClass<Array<int>>()
		.registerClass<Object>();
	try {
		auto a = runtime.create<A>("a").value();
		auto b = runtime.create<B>("b").value();
		auto b2 = runtime.create<B>("b2").value();
		b->a3 = a;
		b->a4 = b2;
		b2->a4 = b;
		auto a2 = b->cast<A>();
		if (!a2) {
			std::cout << a2.error().what() << std::endl;
		}
		json j;
		b->serialize(j);
		std::cout << j.dump(2) << std::endl;
		runtime.collect();
		if (auto r = runtime.deserialize(j)) {
			auto o = r.value();
			json j2;
			o->serialize(j2);
			std::cout << j2.dump(2) << std::endl;
		}
	}
	catch (json::exception& e) {
		std::cout << e.what() << std::endl;
	}
	catch (std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	/*Miyuki::IO::ObjLoadInfo info(nullptr);
	Miyuki::IO::LoadObjFile(R"(C:\Users\xiaoc\source\repos\_MiyukiRenderer\MiyukiRenderer\test-scenes\fireplace_room\mesh0.obj)",
	info); std::cout << info.mtlDescription.dump(2) << std::endl;*/
}