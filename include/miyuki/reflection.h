#ifndef MIYUKI_REFLECTION_H
#define MIYUKI_REFLECTION_H


namespace Miyuki {
	struct TypeInfo {
		struct {
			const char* base;
		} classInfo;
		const char* _name;
		const char* name() {
			return _name;
		}
	};
#define MYK_NULL_CLASS "Null"
#define MYK_DECL_CLASS(classname, basename) \
	TypeInfo typeinfo()const{\
		TypeInfo info;\
		info._name = #classname; \
		info.classInfo.base = #basename;\
		return info; \
	}
}
#endif