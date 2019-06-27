#ifndef MIYUKI_REFLECTION_ARRAY_HPP
#define MIYUKI_REFLECTION_ARRAY_HPP

#include "object.hpp"
#include "primitive.hpp"

namespace Miyuki {
	namespace Reflection {
		template<class T>
		class Array final : public Object {
			using Ty = typename _ConvertToPrimitiveType<T>::type;
			std::vector<Ty*> _array;
		public:
			static Class* __classinfo__() {
				static Class* info = nullptr;
				static std::once_flag flag;
				std::call_once(flag, [&]() {
					info = new Class();
					auto s = new std::string("Miyuki::Reflection::Array<");
					*s += Ty::__classinfo__()->name();
					*s += ">";
					info->_name = s->c_str();
					info->classInfo.size = sizeof(Array<T>);
					info->classInfo.base = Object::__classinfo__();
					info->classInfo.ctor = [=](const UUID& n) {return new Array<T>(n); };
				});
				return info;
			}
			Array(const UUID & id) :Object(__classinfo__(), id) {}
			auto size()const { return _array.size(); }
			void push_back(Ty* t) { _array.push_back(t); }
			void pop_back() { _array.pop_back(); }
			auto begin()const { return _array.begin(); }
			auto begin() { return _array.begin(); }
			auto end()const { return _array.end(); }
			auto end() { return _array.end(); }
			const Ty* at(int i)const {
				return _array.at(i);
			}
			Ty*& at(int i) {
				return _array.at(i);
			}
			void serialize(json& j, SerializationState& state)const override {
				Object::serialize(j, state);
				j["array"] = json::array();
				for (const auto& i : _array) {
					json tmp;
					i->serialize(tmp, state);
					j["array"].push_back(tmp);
				}
			}
			virtual std::vector<Object::Reference> getReferences()override {
				auto result = Object::getReferences();
				int cnt = 0;
				for (auto i : _array) {
					result.emplace_back([=](Object* o)->void {
						_array.at(cnt) = StaticCast<Ty>(o); }, i);
					cnt++;
				}
				return result;
			}

			virtual void deserialize(const json& j, const Resolver& resolve)override {
				Object::deserialize(j, resolve);
				for (const auto& i : j["array"]) {
					if (auto r = resolve(i)) {
						push_back(StaticCast<Ty>(r.value()));
					}
					else {
						throw std::runtime_error(r.error().what());
					}
				}
			}
			bool equals(Object* object)const override {
				if (!object || !isSameType(object))return false;
				auto rhs = StaticCast<Array<T>>(object);
				if (rhs->size() != size())return false;
				for (int i = 0; i < size(); i++) {
					if (at(i) != rhs->at(i))
						return false;
				}
				return true;
			}
		};
	}
}
#endif