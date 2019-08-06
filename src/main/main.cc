#include <ui/mainwindow.h>
#include <math/distribution2d.h>
#include <core/rng.h>
#include <kernel/kernel.h>

void test() {
	using namespace Miyuki;
	using namespace Miyuki::Kernel;
	KernelGlobals globals;
	ShaderProgram program;
	program.program = new Shader * [32];
	program.length = 32;

	FloatShader s1;
	s1.value = 1.0f;
	FloatShader s2;
	s2.value = 0.5f;
	MixedShader s3;
	EndShader s4;
	create_float_shader(&s1);
	create_float_shader(&s2);
	create_mixed_shader(&s3);
	create_end_shader(&s4);
	program.program[0] = (Shader*)& s2;
	program.program[1] = (Shader*)& s1;
	program.program[2] = (Shader*)& s2;
	program.program[3] = (Shader*)& s3;
	program.program[4] = (Shader*)& s4;
	globals.program = program;
	ShadingPoint sp;
	ShaderData data;
	data.offset = 0;
	auto r = svm_eval(&globals, sp, &data);
	fmt::print("{} {} {}\n", r.x, r.y, r.z);
}

int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	//GUI::MainWindow window(argc, argv);
	//window.show();
	test();
	return 0;
}
