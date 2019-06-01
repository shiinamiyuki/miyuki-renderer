#ifndef MIYUKI_CLONEABLE_HPP
#define MIYUKI_CLONEABLE_HPP

#include <miyuki.h>
namespace Miyuki {
	class _ICloneable {};

	template<typename T>
	class ICloneable : public _ICloneable {
		T* _this() {
			return (T*)this;
		}
	public:
		std::unique_ptr<T> clone()const {
			return _this()->cloneImpl();
		}
	};
}

#endif