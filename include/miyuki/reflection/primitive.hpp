#ifndef MIYUKI_REFLECTION_PRIMITIVE_HPP
#define MIYUKI_REFLECTION_PRIMITIVE_HPP

#include "object.hpp"
#include <utils/file.hpp>

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

		class Primitive : public Object {
		public:
			virtual const PrimitiveType primitiveType() const = 0;
			Primitive(Class * _class, const UUID& id)  :Object(_class, id) {}
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
			void init(const T& value) {
				setValue(value);
			}
			virtual const char* type()const {
				return PrimitiveTypeToString(primitiveType());
			}
			bool isPrimitive()const override final { return true; }
			virtual void serialize(json& j, SerializationState&state)const override {
				Object::serialize(j,state);
				j["value"] = value;
			}
			const T& getValue()const {
				return value;
			}
			void setValue(const T& v) { value = v; }
			static Class * __classinfo__() {
				return _primitive_class<T>();
			}
			virtual std::vector<Object::Reference> getReferences()override{
				return {};
			}
			void deserialize(json& j, const Resolver&)override {
				value = j.at("value").get<T>();
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
		//template<typename T>
		//struct LeafNodeDeserializer : public IDeserializer {
		//	virtual Node* deserialize(const json& j, Graph* G) {
		//		// skip type checking
		//		return new LeafNode<T>(j.at("name").get<std::string>(), j.at("value").get<T>(), G);
		//	}
		//};


		using IntNode = PrimitiveT<int>;
		using FloatNode = PrimitiveT<Float>;
		using Float3Node = PrimitiveT<Vec3f>;
		using StringNode = PrimitiveT<std::string>;
		using FileNode = PrimitiveT<File>;
		template<typename T>
		struct _ConvertToPrimitiveType {
			using type = typename  std::conditional<_GetLeafType<T>::Type == kNull, T, PrimitiveT<T>>::type;
		};
//#define MYK_CLASS(Classname) virtual const char* type()const override{return #Classname;}

	}
}
#endif