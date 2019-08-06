#ifndef MIYUKI_KERNEL_SVM_H
#define MIYUKI_KERNEL_SVM_H

#include "kerneldef.h"

MYK_KERNEL_NS_BEGIN

#define MYK_SVM_STACK_DEPTH 64




// Shader Virtual Machine
typedef struct SVM {
	ShadingResult stack[MYK_SVM_STACK_DEPTH];
	int sp;
}SVM;
MYK_KERNEL_FUNC void svm_init(SVM* svm) {
	svm->sp = 0;
}
MYK_KERNEL_FUNC void svm_push(SVM* svm, ShadingResult value) {
	if (sp >= MYK_SVM_STACK_DEPTH) {
		MYK_KERNEL_PANIC("maxium stack depth reached");
	}
	svm->stack[sp++] = value;
}
MYK_KERNEL_FUNC ShadingResult svm_pop(SVM* svm) {
	if (sp <= 0) {
		MYK_KERNEL_PANIC("stack underflow");
	}
	svm->stack[--sp] = value;
}
MYK_KERNEL_NS_END

#endif