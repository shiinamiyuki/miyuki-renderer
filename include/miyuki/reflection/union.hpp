#include <reflection/reflection.hpp>

namespace Miyuki {
	namespace Reflection {
		template<typename T, class _Base = Reflection::Object>
		class Union : public _Base {
			using Ty = typename _ConvertToPrimitiveType<T>::type;
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					auto s = new std::string("Miyuki::Reflection::Union<");
					*s += T::__classinfo__()->name();
					*s += ">";
					info->_name = s->c_str();
					info->classInfo.size = sizeof(Union<Ty,_Base>);
					info->classInfo.base = _Base::__classinfo__();
					info->classInfo.ctor = [=](const UUID& n) {return new Union<Ty, _Base>(n); };
				});
				return info;
			}
			Union(const UUID & id) :_Base(__classinfo__(), id) {}
		protected:
			
			Miyuki::Reflection::PropertyT<Ty> ptr;
		public:
			void init(Ty* p) {
				set(p);
			}
			Ty* get() {
				return ptr.get();
			}
			const Ty* get()const {
				return ptr.get();
			}
			void set(Ty* p) {
				ptr = p;
			}
			const std::vector<const Miyuki::Reflection::Property*> getProperties()const override {
				std::vector<const Miyuki::Reflection::Property*> vec;
				vec.emplace_back(&ptr);
				return vec;
			}
		};
	}
}