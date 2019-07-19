#include <miyuki.h>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/algorithm/string/replace.hpp>

template<class T>
struct Miyuki_MetaInfo {
	template<class _1, class _2>
	static void accept(_1, _2) {}
};

namespace Miyuki {
	namespace Reflection {

		template<int>
		struct UID {};
		template<class T, class Visitor>
		void visit(T& x, Visitor& v) {
			using _T = std::decay_t<T>;
			Miyuki_MetaInfo<_T>::accept(x, v);
		}
		template<class T>
		struct IsObject {
			static const int value = false;
		};


		template<class T>
		using MetaInfo = Miyuki_MetaInfo<T>;


		class Object;


		class Reference {
			Object* object = nullptr;
			std::function<void(Object*)> resetFunc;
		public:
			Reference(Object* object, std::function<void(Object*)> resetFunc) :object(object), resetFunc(resetFunc) {}
			Reference(Object* object) :object(object) {}
			Object* get() { return object; }
			void set(Object* o) { resetFunc(o); }
		};

		class OutObjectStream {
			json data;
			struct State {
				std::set<const Object*> visited;
			};
			std::shared_ptr<State> state;
		public:
			OutObjectStream() {
				state = std::make_shared<State>();
			}
			OutObjectStream sub() {
				OutObjectStream stream;
				stream.state = state;
				return stream;
			}
			void write(const std::string& key, std::nullptr_t) {
				data[key] = {};
			}
			void write(const std::string& key, const OutObjectStream& stream) {
				data[key] = stream.data;
			}
			void write(const std::string& key, const std::string& value) {
				data[key] = value;
			}
			void write(const std::string& key, size_t value) {
				data[key] = value;
			}
			void append(const OutObjectStream& stream) {
				data.push_back(stream.data);
			}
			void append(std::nullptr_t) {
				data.push_back(json{});
			}
			bool hasSerialized(const Object* object)const {
				return state->visited.find(object) != state->visited.end();
			}
			void addSerialized(const Object* object) {
				state->visited.insert(object);
			}
			void write(int value) {
				data = value;
			}
			void write(const std::string& value) {
				data = value;
			}
			void write(float value) {
				data = value;
			}
			void write(double value) {
				data = value;
			}
			void write(int64_t value) {
				data = value;
			}
			void write(uint64_t value) {
				data = value;
			}
			void write(uint32_t value) {
				data = value;
			}
			void write(const Vec3f& value) {
				data = value;
			}
			void write(const Point2i& value) {
				data = value;
			}
			void write(const Point2f& value) {
				data = value;
			}
			void write(const Point3f& value) {
				data = value;
			}
			void writeReference(const Object* ref) {
				data = json::object();
				data["ref"] = std::to_string(reinterpret_cast<size_t>(ref));
			}
			const json& toJson()const { return data; }
		};
		class Runtime;
		class InObjectStream {
			friend class Runtime;
			const json& data;
			struct State {
				std::unordered_map<size_t, Object*> map;
			};
			std::shared_ptr<State> state;
		public:
			InObjectStream(const json& data, std::shared_ptr<State>s = nullptr) :data(data) {
				if (!s)
					state = std::make_shared<State>();
				else
					state = s;
			}
			InObjectStream sub(const std::string& key) {
				return InObjectStream(data.at(key), state);
			}
			InObjectStream sub(size_t index) {
				return InObjectStream(data.at(index), state);
			}
			bool contains(const std::string& key) {
				return data.contains(key);
			}
			const json& getJson() {
				return data;
			}
			Object* fetchByAddr(size_t addr) {
				return state->map.at(addr);
			}
			size_t size() { return data.size(); }
		};
		class GCContext;
#define MYK_SAVE_TRIVIAL(type)\
		void save(const type& value, OutObjectStream& stream) {\
			stream.write(value);\
		}
#define MYK_LOAD_TRIVIAL(type)\
		void load(type& value, InObjectStream& stream) {\
			value = stream.getJson().get<type>();\
		}

#define MYK_SAVE_LOAD_TRIVIAL(type) 	MYK_SAVE_TRIVIAL(type)MYK_LOAD_TRIVIAL(type)
		MYK_SAVE_LOAD_TRIVIAL(int32_t);
		MYK_SAVE_LOAD_TRIVIAL(uint32_t);
		MYK_SAVE_LOAD_TRIVIAL(uint64_t);
		MYK_SAVE_LOAD_TRIVIAL(int64_t);
		MYK_SAVE_LOAD_TRIVIAL(int8_t);
		MYK_SAVE_LOAD_TRIVIAL(char);
		MYK_SAVE_LOAD_TRIVIAL(uint8_t);
		MYK_SAVE_LOAD_TRIVIAL(Float);
		MYK_SAVE_LOAD_TRIVIAL(double);
		MYK_SAVE_LOAD_TRIVIAL(Vec3f);
		MYK_SAVE_LOAD_TRIVIAL(Spectrum);
		MYK_SAVE_LOAD_TRIVIAL(std::string);
		MYK_SAVE_LOAD_TRIVIAL(Point2i);
		MYK_SAVE_LOAD_TRIVIAL(Point2f);
		MYK_SAVE_LOAD_TRIVIAL(Point3f);


		struct OutStreamVisitor {
			OutObjectStream& stream;
			OutStreamVisitor(OutObjectStream& stream) :stream(stream) { }
			template<class T>
			void visit(const T& value, const char* name) {
				auto sub = stream.sub();
				save(value, sub);
				stream.write(name, sub);
			}
		};

		struct InStreamVisitor {
			int index = 0;
			InObjectStream& stream;
			InStreamVisitor(InObjectStream& stream) :stream(stream) {}
			template<class T>
			void visit(T& value, const char* name) {
				if (stream.contains(name)) {
					auto sub = stream.sub(name);
					load(value, sub);
				}
			}
		};

		template<class T>
		std::enable_if_t<std::is_base_of_v<Object, T>, void>
			save(T* object, OutObjectStream& stream) {
			if (!object) {
				stream.write("ref", nullptr);
				return;
			}
			if (!object->isManaged()) {
				throw std::runtime_error("Cannot have pointer to a non-managed object");
			}
			object->serialize(stream);

		}

		template<class T>
		std::enable_if_t<std::is_base_of_v<Object, T>, void>
			save(const T& object, OutObjectStream& stream) {
			stream.write("type", object.getClass()->getName());
			object.serializeInternal(stream);

		}

		template<class T>
		void save(const std::vector<T>& vec, OutObjectStream& stream) {
			for (const auto& item : vec) {
				auto sub = stream.sub();
				save(item, sub);
				stream.append(sub);
			}
		}
		template<class T>
		void load(std::vector<T>& vec, InObjectStream& stream) {
			for (auto i = 0; i < stream.size(); i++) {
				auto sub = stream.sub(i);
				T value;
				load(value, sub);
				vec.emplace_back(std::move(value));
			}
		}

		template<class K, class V>
		void save(const std::unordered_map<K, V>& map, OutObjectStream& stream) {
			for (const auto& item : map) {
				auto pair = stream.sub();
				auto key = pair.sub();
				auto val = pair.sub();
				save(item.first, key);
				save(item.second, val);
				pair.write("key", key);
				pair.write("val", val);
				stream.append(pair);
			}
		}
		template<class K, class V>
		void load(std::unordered_map<K, V>& map, InObjectStream& stream) {
			for (auto i = 0; i < stream.size(); i++) {
				auto pair = stream.sub(i);
				K key;
				V val;
				load(key, pair.sub("key"));
				load(val, pair.sub("val"));
				map[key] = val;
			}
		}
		template<class K, class V>
		void save(const std::map<K, V>& map, OutObjectStream& stream) {
			for (const auto& item : map) {
				auto pair = stream.sub();
				auto key = pair.sub();
				auto val = pair.sub();
				save(item.first, key);
				save(item.second, val);
				pair.write("key", key);
				pair.write("val", val);
				stream.append(pair);
			}
		}
		template<class K, class V>
		void load(std::map<K, V>& map, InObjectStream& stream) {
			for (auto i = 0; i < stream.size(); i++) {
				auto pair = stream.sub(i);
				K key;
				V val;
				load(key, pair.sub("key"));
				load(val, pair.sub("val"));
				map[key] = val;
			}
		}

		template<class T>
		std::enable_if_t<std::is_convertible_v<T*, Object*>, void>
			load(T& object, InObjectStream& stream) {
			object.deserialize(stream);
		}

		template<class T>
		std::enable_if_t<std::is_convertible_v<T*, Object*>, void>
			load(T*& object, InObjectStream& stream) {
			auto& data = stream.getJson();
			if (data.contains("ref")) {
				if (data["ref"].is_null())
					object = nullptr;
				else
					object = stream.fetchByAddr(std::stoull(data["ref"].get<std::string>()))->cast<T>();
			}
			else {
				object = stream.fetchByAddr(std::stoull(data["addr"].get<std::string>()))->cast<T>();
				object->deserialize(stream);
			}

		}
		class GCContext {
		public:
			void addReference(const Object* object) {
			}
		};

		struct GCReferenceFindingVisitor {
			GCContext& ctx;
			GCReferenceFindingVisitor(GCContext& ctx) :ctx(ctx) {}
			template<class T>
			void visit(const T& value, const char* name) {
				findReferences(value, ctx);
			}
		};

		template<class T>
		std::enable_if_t<std::is_base_of_v<Object, T>, void>
			findReferences(const T* value, GCContext& ctx){
			ctx.addReference(value);
			GCReferenceFindingVisitor visitor(ctx);
			visit(*value, visitor);
		}

		template<class T>
		void findReferences(const std::vector<T>& value, GCContext& ctx) {
			for (const auto& item : value) {
				findReferences(item, ctx);
			}
		}

		template<class K,class V>
		void findReferences(const std::map<K, V>& value, GCContext& ctx) {
			for (const auto& item : value) {
				findReferences(item.first, ctx);
				findReferences(item.second, ctx);
			}
		}

		template<class K, class V>
		void findReferences(const std::unordered_map<K, V>& value, GCContext& ctx) {
			for (const auto& item : value) {
				findReferences(item.first, ctx);
				findReferences(item.second, ctx);
			}
		}


		void findReferences(...) {}

		class PropertyVisitor {
		public:
			virtual void visit(int32_t) = 0;
			virtual void visit(int32_t&, const char* name) = 0;
		};
		class Class {
		public:
			const char* name;
			std::function<Object* ()> ctor;
			Class* base = nullptr;
			std::set<Class* > derived;
			std::function<void(Object*, OutObjectStream&)> serializer;

			Class(Class* base, const char* name, std::function<Object* ()> ctor)
				:base(base), name(name), ctor(std::move(ctor)) {}
			const char* getName() { return name; }
			Object* create()const {
				return ctor();
			}
			void addDerived(Class* info) { derived.insert(info); }
			Class* getBase()const {
				return base;
			}
		};

		class ClassCastException : public std::exception {
			std::string msg;
		public:
			ClassCastException(Class* from, Class* to) {
				msg = std::string("ClassCastException:  Cannot cast from ").append(from->getName()).append(" to ").append(to->getName());
			}
			const char* what()const {
				return msg.c_str();
			}
		};
		class Runtime;
		class Object {
			Class* _class;
			friend class Runtime;
		protected:

		public:
			virtual void serializeInternal(OutObjectStream& stream)const {}
			virtual void deserializeInternal(InObjectStream& stream) {}
			static Class* getStaticClass() {
				static std::once_flag flag;
				static Class* info;
				std::call_once(flag, [&]() {
					info = new Class(nullptr, "Miyuki::Reflection::Object", []() {return nullptr; });
					info->serializer = [](Object* object, OutObjectStream& stream) {

					};
				});
				return info;
			}
			Class* getClass()const {
				return _class;
			}
			virtual void serialize(OutObjectStream& stream) const = 0;
			virtual void deserialize(InObjectStream& stream) = 0;
			virtual void visitReferences(GCContext& ctx)const = 0;
			virtual ~Object() {}
			template<class T>
			T* cast() {
				auto _class = getClass();
				while (_class) {
					if (_class == T::getStaticClass())
						return static_cast<T*>(this);
					_class = _class->getBase();
				}
				throw ClassCastException(getClass(), T::getStaticClass());
			}
			template<class T>
			const T* cast()const {
				auto _class = getClass();
				while (_class) {
					if (_class == T::getStaticClass())
						return static_cast<T*>(this);
					_class = _class->getBase();
				}
				throw ClassCastException(getClass(), T::getStaticClass());
			}
			Object(Class* _class) :_class(_class) {}
			inline bool isManaged()const;
		};
		template<>
		struct Miyuki_MetaInfo<Object> {
		};
		template<>
		struct IsObject<Object> {
			static const int value = true;
		};
		template<class T, class Base>
		struct GetClassInfo {
			static Class* GetClass(const char* name) {
				static std::once_flag flag;
				static Class* info;
				std::call_once(flag, [&]() {
					info = new Class(Base::getStaticClass(), name, []() {return new T(); });
					info->serializer = [](Object* object, OutObjectStream& stream) {
						static_cast<T*>(object)->serialize(stream);
					};
				});
				return info;
			}
		};

#define MYK_CLASS(Classname, Super) using __Self = Classname;using __Super = Super;friend struct Miyuki_MetaInfo<__Self>; \
		using base = __Super;using super = __Super;static inline Miyuki::Reflection::Class * getStaticClass();\
		Classname(Miyuki::Reflection::Class * info):base(info){}\
		Classname() :__Super(__Self::getStaticClass()) {}\
		virtual void serializeInternal(Miyuki::Reflection::OutObjectStream& stream)const override {\
				auto _base = stream.sub(); \
				auto _this = stream.sub(); \
				base::serializeInternal(_base); \
				Miyuki::Reflection::OutStreamVisitor visitor(_this);\
				Miyuki::Reflection::visit(*this, visitor); \
				stream.write("base", _base); \
				stream.write("this", _this); \
		}\
		virtual void serialize(Miyuki::Reflection::OutObjectStream& stream)const override {\
			if(stream.hasSerialized(this)){\
				stream.writeReference(static_cast<const Miyuki::Reflection::Object*>(this));\
			}\
			else {\
				stream.addSerialized(static_cast<const Miyuki::Reflection::Object*>(this));\
				serializeInternal(stream);\
				stream.write("type", getClass()->getName());\
				if(Miyuki::Reflection::IsManaged(this))\
					stream.write("addr", std::to_string(reinterpret_cast<size_t>(static_cast<const Miyuki::Reflection::Object*>(this))));\
			}\
		}\
		virtual void deserializeInternal(Miyuki::Reflection::InObjectStream& stream)override {\
			base::deserializeInternal(stream.sub("base"));\
			auto _this = stream.sub("this");\
			Miyuki::Reflection::InStreamVisitor visitor(_this);\
			Miyuki::Reflection::visit(*this, visitor); \
		}\
		virtual void deserialize(Miyuki::Reflection::InObjectStream& stream)override {\
			if (getClass()->getName() != stream.getJson().at("type").get<std::string>()) {\
				throw std::runtime_error("Type mismatch");\
			}\
			deserializeInternal(stream);\
		}\
		virtual void visitReferences(Miyuki::Reflection::GCContext&ctx)const override{\
			findReferences(this, ctx);\
		}


		inline Class* typeof(Object* object) {
			return object ? object->getClass() : nullptr;
		}

		class Runtime {
			std::unordered_map<std::string, Class*> classes;
			static Runtime* instance;
			static std::once_flag flag;
			std::set<Object*> allObjects;

			Object* createObject(const json& data) {
				auto type = data.at("type").get<std::string>();
				auto _class = classes.at(type);
				auto object = _class->ctor();
				addObject(object);
				return object;
			}

			/*
			Scan the entire stream and create all managed objects
			*/
			void createAllObjects(InObjectStream& in) {
				auto& data = in.getJson();
				if (data.is_array()) {
					for (size_t i = 0; i < in.size(); i++) {
						createAllObjects(in.sub(i));
					}
				}
				if (!data.is_object() || data.is_null())return;
				if (data.contains("ref"))
					return;
				if (data.contains("this")) {
					if (data.contains("addr")) { // is managed
						auto addr = std::stoull(data["addr"].get<std::string>());
						auto object = createObject(data);
						in.state->map.insert(std::make_pair(addr, object));
					}
					for (auto& el : data.at("this").items()) {
						auto sub = in.sub("this").sub(el.key());
						createAllObjects(sub);
					}
					if (!data.at("base").is_null()) {
						auto sub = in.sub("base");
						createAllObjects(sub);
					}
				}
				else {
					for (auto& el : data.items()) {
						createAllObjects(in.sub(el.key()));
					}
				}
			}

			Object* deserializeObject(InObjectStream& in) {
				auto& data = in.getJson();
				Object* object;
				if (data.contains("ref")) {
					throw std::runtime_error("Object graph root must not be a ref link");
				}
				else {
					if (data.contains("addr")) {
						auto addr = std::stoull(data.at("addr").get<std::string>());
						object = in.fetchByAddr(addr);
						object->deserialize(in);
						return object;
					}
					else {
						throw std::runtime_error("Object graph root must be managed");
					}
				}
			}

		public:
			template<class T>
			T* readObject(InObjectStream& in) {
				createAllObjects(in);
				auto object = deserializeObject(in);
				return object->cast<T>();
			}
			void addClass(Class* info) {
				classes[info->getName()] = info;
				info->getBase()->addDerived(info);
			}
			static Runtime& GetInstance() {
				std::call_once(flag, [&]() {
					instance = new Runtime();
				});
				return *instance;
			}
			template<class T>
			void addObject(T* object) {
				allObjects.insert(static_cast<Object*>(object));
			}
			bool isManaged(const Object* object)const {
				return allObjects.find(const_cast<Object*>(object)) != allObjects.end();
			}

			Class* getClass(const std::string& s) {
				return classes.at(s);
			}
		};

		inline bool Object::isManaged()const {
			return Runtime::GetInstance().isManaged(this);
		}
		template<class T>
		inline void registerType() {
			auto t = T::getStaticClass();
			auto& instance = Runtime::GetInstance();
			instance.addClass(t);
		}
		bool IsManaged(const Object* object) {
			auto& instance = Runtime::GetInstance();
			return instance.isManaged(object);
		}

		template<class T = Object>
		T * NewObject() {
			auto& instance = Runtime::GetInstance();
			auto object = new T();
			auto ptr = object->cast<T>();
			instance.addObject(object);
			return ptr;
		}
		template<class T = Object>
		T * NewObject(Class * _class) {
			auto& instance = Runtime::GetInstance();
			auto object = _class->ctor();
			auto ptr = object->cast<T>();
			instance.addObject(object);
			return ptr;
		}
		template<class T>
		T* ReadObject(InObjectStream& in) {
			auto& instance = Runtime::GetInstance();
			return instance.readObject<T>(in);
		}
		Class* GetClass(std::string s) {
			boost::replace_all(s, ".", "::");
			return Runtime::GetInstance().getClass(s);
		}
	}
	struct __Injector {
		__Injector(std::function<void(void)> f) { f(); }
	};
}
#define MYK_AUTO_REGSITER_TYPE(Type) \
static Miyuki::__Injector BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(injector,__COUNTER__),_),__LINE__ )\
											([](){Miyuki::Reflection::registerType<Type>();});\
inline Miyuki::Reflection::Class * Type::getStaticClass(){\
	return Miyuki::Reflection::GetClassInfo<Type,Type::base>::GetClass(#Type);\
}



#define MYK_BEGIN_REFL(Type) MYK_AUTO_REGSITER_TYPE(Type)template<>struct Miyuki_MetaInfo<Type> {\
						 enum {__idx = __COUNTER__}; using __Self = Type;\
						template<int i>using UID = Miyuki::Reflection::UID<i>;static constexpr char * TypeName = #Type;

#define MYK_PROP(name)  enum {__attr_index_##name = __COUNTER__ - __idx - 1 };\
						static auto& getProperty(__Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto& getProperty(const __Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto getPropertyName(UID<__attr_index_##name>) { return #name; }
#define MYK_END_REFL  enum{__attr_count =  __COUNTER__ - __idx - 1 };\
					template<class SelfT, class Visitor>\
					static void accept(SelfT& self,Visitor& visitor){ \
						__AcceptHelper<SelfT, Visitor, __attr_count - 1>::accept(self, visitor);\
					}\
					template<class SelfT, class Visitor, int i>\
					struct __AcceptHelper {\
						using Meta = Miyuki::Reflection::MetaInfo<__Self>;\
						static void accept(SelfT& self, Visitor& visitor) {\
							visitor.visit(Meta::getProperty(self, UID<Meta::__attr_count - i - 1>()), Meta::getPropertyName(UID<Meta::__attr_count - i - 1>()));\
							__AcceptHelper<SelfT, Visitor, i - 1>::accept(self, visitor);\
						}\
					};\
					template<class SelfT, class Visitor>\
					struct __AcceptHelper<SelfT, Visitor, 0> {\
						using Meta = Miyuki::Reflection::MetaInfo<__Self>;\
						static void accept(SelfT& self, Visitor& visitor) {\
							visitor.visit(Meta::getProperty(self, UID<Meta::__attr_count - 1>()), Meta::getPropertyName(UID<Meta::__attr_count - 1>()));\
						}\
					};};
#define MYK_SEQ_MACRO(r, data, elem) MYK_PROP(elem)
#define MYK_REFL(Type, Attributes) MYK_BEGIN_REFL(Type) BOOST_PP_SEQ_FOR_EACH(MYK_SEQ_MACRO, _, Attributes) MYK_END_REFL

#define MYK_PRIMITIVE_CLASS(Type, pType) \
		namespace Miyuki{namespace Reflection{class Type : public Miyuki::Reflection::Object {\
			pType value = pType();\
		public:\
			MYK_CLASS(Type, Miyuki::Reflection::Object);\
			pType& get(){return value;}\
			const pType& get()const { return value; }\
		};\
		Type* Box(const pType& value) {\
			auto object = NewObject<Type>();\
			object->get() = value;\
			return object;\
		}}}MYK_REFL(Miyuki::Reflection::Type, (value))
		

MYK_PRIMITIVE_CLASS(Int8, char);
MYK_PRIMITIVE_CLASS(UInt8, uint8_t);
MYK_PRIMITIVE_CLASS(Int32, int);
MYK_PRIMITIVE_CLASS(UInt32, uint32_t);
MYK_PRIMITIVE_CLASS(Int64, int64_t);
MYK_PRIMITIVE_CLASS(UInt64, uint64_t);
MYK_PRIMITIVE_CLASS(Float64, double);
MYK_PRIMITIVE_CLASS(Float32, float);


class A : public Miyuki::Reflection::Object {
public:
	MYK_CLASS(A, Miyuki::Reflection::Object);
	int a=0;
	virtual std::vector< Miyuki::Reflection::Reference> getReferences() { return {}; }
};
MYK_REFL(A, (a))
class B : public A {
public:
	A a2;
	B* next=nullptr;
	std::unordered_map<B*, int> map;
	MYK_CLASS(B, A)
};

MYK_REFL(B, (a2)(next)(map))

Miyuki::Reflection::Runtime* Miyuki::Reflection::Runtime::instance;
std::once_flag Miyuki::Reflection::Runtime::flag;
int main(int argc, char** argv) {
	using namespace Miyuki;
	using namespace Miyuki::Reflection;
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	try {
		auto b = NewObject<B>();
		b->a2.a = 5;
		b->a = 1;
		b->next = nullptr;
		b->map[b] = 3;
		b->next = NewObject<B>();
		auto tmp = NewObject<B>();
		b->map[tmp] = 4;
		b->next->next = b;
		b->next->a = 2;
		OutObjectStream stream;
		b->serialize(stream);
		auto j = stream.toJson();
		std::cout << j.dump(1) << std::endl;
		InObjectStream in(j);
		auto b2 = ReadObject<B>(in);
		OutObjectStream s2;
		b2->serialize(s2);

		std::cout << s2.toJson().dump(1) << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
