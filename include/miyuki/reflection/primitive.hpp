#ifndef MIYUKI_REFLECTION_PRIMITIVE_HPP
#define MIYUKI_REFLECTION_PRIMITIVE_HPP

#include "object.hpp"

namespace Miyuki {
	namespace Reflection {
		enum PrimitiveType {
			kNull,
			kInt,
			kFloat,
			kFloat3,
			kString,
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
			}
			return nullptr;
		}

		class Primitive : public Object {
		public:
			virtual const PrimitiveType primitiveType() const = 0;
			Primitive(const Class * _class)  :Object(_class) {}
			const std::vector<Property*> getProperties()const override { return {}; }
		};

		template<typename T>
		class PrimitiveT : public Primitive {
			T value;
		public:
			PrimitiveT(const T& value, Graph* graph = nullptr) :
				Primitive("", graph), value(value) {}
			PrimitiveT(const std::string& name, const T& value, Graph* graph = nullptr) :
				Primitive(name, graph), value(value) {}
			virtual const PrimitiveType primitiveType() const override {
				return PrimitiveType(_GetLeafType<T>::Type);
			}
			virtual const char* type()const {
				return PrimitiveTypeToString(primitiveType());
			}
			bool isLeaf()const override final { return true; }
			virtual void serialize(json& j)const override {
				Object::serialize(j);
				j["value"] = value;
			}
			const T& getValue()const {
				return value;
			}
			void setValue(const T& v) { value = v; }
		};

		template<typename T>
		struct LeafNodeDeserializer : public IDeserializer {
			virtual Node* deserialize(const json& j, Graph* G) {
				// skip type checking
				return new LeafNode<T>(j.at("name").get<std::string>(), j.at("value").get<T>(), G);
			}
		};


		using IntNode = PrimitiveT<int>;
		using FloatNode = PrimitiveT<Float>;
		using Float3Node = PrimitiveT<Vec3f>;
		using StringNode = PrimitiveT<std::string>;
		template<typename T>
		struct _ConvertToPrimitiveType {
			using type = typename  std::conditional<_GetLeafType<T>::Type == kNull, T, LeafNode<T>>::type;
		};
//#define MYK_CLASS(Classname) virtual const char* type()const override{return #Classname;}

	}
}
#endif