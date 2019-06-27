#ifndef MIYUKI_REFLECTION_PRIMITIVE_HPP
#define MIYUKI_REFLECTION_PRIMITIVE_HPP

#include "object.hpp"
#include <utils/file.hpp>
#include <boost/functional/hash.hpp>
namespace std {
	template<>struct hash<Miyuki::Vec3f> {
		typedef Miyuki::Vec3f argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept
		{
			result_type h(std::hash<float>{}(s[0]));
			boost::hash_combine(h, s[1]);
			boost::hash_combine(h, s[2]);
			return h;
		}
	};
}
namespace Miyuki {
	namespace Reflection {
		enum PrimitiveType {
			kNull,
			kInt,
			kFloat,
			kFloat3,
			kString,
			kFile
		};

		template<typename T>
		struct _GetLeafType {
			static constexpr int Type = kNull;
		};

		template<>
		struct _GetLeafType<Float> {
			static constexpr int Type = kFloat;
		};

		template<>
		struct _GetLeafType<Vec3f> {
			static constexpr int Type = kFloat3;
		};

		template<>
		struct _GetLeafType<int> {
			static constexpr int Type = kInt;
		};

		template<>
		struct _GetLeafType<std::string> {
			static constexpr int Type = kString;
		};

		template<>
		struct _GetLeafType<File> {
			static constexpr int Type = kFile;
		};

		inline const char* PrimitiveTypeToString(PrimitiveType t) {
			switch (t) {
			case kString:
				return "String";
			case kFloat:
				return "Float";
			case kInt:
				return "Int";
			case kFloat3:
				return "Float3";
			case kFile:
				return "File";
			}
			return nullptr;
		}

		template<class T>
		struct Hash {
			size_t operator()(const T& v)const {
				return std::hash<T>{}(v);
			}
		};
		class Primitive : public Object {
		public:
			virtual const PrimitiveType primitiveType() const = 0;
			Primitive(Class* _class, const UUID& id) :Object(_class, id) {}
		};

		template<class T>
		inline Class* _primitive_class() {
			return nullptr;
		}

		template<typename T>
		class PrimitiveT : public Primitive {
			T value = T();
		public:
			PrimitiveT(const UUID& id) :
				Primitive(_primitive_class<T>(), id) {}
			PrimitiveT(const T& value, const UUID& id) :
				Primitive(_primitive_class<T>(), id), value(value) {}
			virtual const PrimitiveType primitiveType() const override {
				return PrimitiveType(_GetLeafType<T>::Type);
			}
			void init() {
				value = T();
			}
			void init(const T& value) {
				setValue(value);
			}
			virtual const char* type()const {
				return PrimitiveTypeToString(primitiveType());
			}
			bool isPrimitive()const override final { return true; }
			virtual void serialize(json& j, SerializationState& state)const override {
				Object::serialize(j, state);
				j["value"] = value;
			}
			const T& getValue()const {
				return value;
			}
			void setValue(const T& v) { value = v; }
			static Class* __classinfo__() {
				return _primitive_class<T>();
			}
			virtual std::vector<Object::Reference> getReferences()override {
				return {};
			}
			virtual void deserialize(const json& j, const Resolver& resolve)override {
				value = j.at("value").get<T>();
			}
			bool equals(Object* val)const  override {
				if (!val || !isSameType(val))return false;
				auto rhs = StaticCast<PrimitiveT<T>>(val);
				return value == rhs->value;
			}
			size_t hashCode()const override {
				return Hash<T>()(value);
			}
		};
		template<>
		struct Hash<File> {
			size_t operator()(const File& file)const {
				return 0;
			}
		};
#define _MYK_PRIMITIVE_CLASS(Ty)	template<> \
								inline Class* _primitive_class<Ty>() { \
									static Class* info = nullptr; \
										static std::once_flag flag;\
										std::call_once(flag, [&]() {info = new Class();  \
										info->_name = "Primitive::" #Ty; \
										info->classInfo.base = nullptr; \
										info->classInfo.ctor = [=](const UUID& n) {return new PrimitiveT<Ty>(n); };});\
									return info; \
								}
		_MYK_PRIMITIVE_CLASS(int)
		_MYK_PRIMITIVE_CLASS(Float)
		_MYK_PRIMITIVE_CLASS(Vec3f)
		_MYK_PRIMITIVE_CLASS(std::string)
		_MYK_PRIMITIVE_CLASS(File)

		using IntNode = PrimitiveT<int>;
		using FloatNode = PrimitiveT<Float>;
		using Float3Node = PrimitiveT<Vec3f>;
		using StringNode = PrimitiveT<std::string>;
		using FileNode = PrimitiveT<File>;
		template<typename T>
		struct _ConvertToPrimitiveType {
			using type = typename  std::conditional<_GetLeafType<T>::Type == kNull, T, PrimitiveT<T>>::type;
		};
	}
}
#endif