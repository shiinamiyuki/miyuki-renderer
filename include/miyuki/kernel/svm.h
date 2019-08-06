#ifndef MIYUKI_KERNEL_SVM_H
#define MIYUKI_KERNEL_SVM_H

#include "kerneldef.h"
#include "shaderdata.h"
MYK_KERNEL_NS_BEGIN

#define MYK_SVM_STACK_DEPTH 64


// Shader Virtual Machine
typedef struct SVM {
	ShadingResult stack[MYK_SVM_STACK_DEPTH];
	ShadingPoint shading_point;
	int sp;
	int pc;	
	bool halt;
}SVM;

MYK_KERNEL_FUNC void svm_init(SVM* svm, int pc, ShadingPoint sp) {
	svm->sp = 0;
	svm->pc = pc;
	svm->shading_point = sp;
	svm->halt = false;
}
MYK_KERNEL_FUNC_INLINE void svm_pc_advance(SVM* svm) {
	svm->pc++;
}

MYK_KERNEL_FUNC_INLINE 
void svm_branch_if(SVM* svm, bool value, int offset) {
	if (value) {
		svm->pc+= offset - 1;
	}
}

MYK_KERNEL_FUNC_INLINE
void svm_jump(SVM* svm, int offset) {
	svm->pc += offset - 1;
}

MYK_KERNEL_FUNC_INLINE void svm_push(SVM* svm, ShadingResult value) {
	if (svm->sp >= MYK_SVM_STACK_DEPTH) {
		MYK_KERNEL_PANIC("maxium stack depth reached");
	}
	svm->stack[svm->sp++] = value;
}
MYK_KERNEL_FUNC_INLINE ShadingResult svm_pop(SVM* svm) {
	if (svm->sp <= 0) {
		MYK_KERNEL_PANIC("stack underflow");
	}
	return svm->stack[--svm->sp];
}

MYK_KERNEL_FUNC_INLINE
Shader* read_next_shader(KernelGlobals*globals, SVM* svm) {
	return globals->program.program[svm->pc];
}

MYK_KERNEL_FUNC
ShadingResult svm_eval(KernelGlobals* globals, ShadingPoint sp, ShaderData* data);
MYK_KERNEL_NS_END

#endif