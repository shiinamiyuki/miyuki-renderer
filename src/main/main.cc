#include <ui/mainwindow.h>
//#include <reflection.h>
//#include <graph/materialnode.h>
//#include <io/importobj.h>
//#include <utils/future.hpp>


namespace Miyuki {
	namespace Reflection {
		struct Trait;
		struct Deleter {
			std::function<void(Trait*)> deleter;
			Deleter() {}
			Deleter(std::function<void(Trait*)> f) :deleter(std::move(f)) {}
			void operator()(Trait* t) {
				deleter(t);
			}
		};
		template<class T = Trait>
		using Box = std::unique_ptr<T, Deleter>;

		template<class T, class... Args>
		Box<T> make_box(Args... args) {
			return Box<T>(new T(args...), Deleter([](Trait* p) {delete p; }));
		}
		struct OutStream {
			struct State {
				std::set<const void*> _visitedPtr;
				bool visited(const void* ptr) {
					return _visitedPtr.find(ptr) != _visitedPtr.end();
				}

			};
			json data;
			State* state;
			OutStream() {
				_stateOwner = std::make_unique<State>();
				state = _stateOwner.get();
			}
			OutStream(const OutStream& rhs) :state(rhs.state) {}
			OutStream& operator = (const OutStream& rhs) {
				state = rhs.state;
				return *this;
			}
			void from(const OutStream& rhs) {
				data = rhs.data;
				CHECK(state == rhs.state);
			}
			template<class T>
			bool visited(const T* ptr) {
				return state->visited(ptr);
			}
			void addVisited(const void* ptr) {
				state->_visitedPtr.insert(ptr);
			}
		private:
			std::unique_ptr<State> _stateOwner;
		};


		template<class T>
		struct Saver {
			static void save(const T& value, OutStream& stream) {
				OutStreamVisitor visitor;
				visitor.stream = stream;
				visitor.stream.data = json::array();
				Miyuki::Reflection::visit(value, visitor);
				stream.data = json::object();
				stream.data["type"] = T::type()->name();
				stream.data["val"] = visitor.stream.data;
			}
		};

		struct OutStreamVisitor {
			OutStream stream;
			OutStreamVisitor() {}
			OutStreamVisitor(const OutStream& stream) :stream(stream) { }
			template<class T>
			void visit(const T& value, const char* name) {
				OutStream s(stream);
				Saver<T>::save(value, s);
				stream.data.push_back(s.data);
			}
		};
#define MYK_SAVE_TRIVIAL(T)\
		template<>\
		struct Saver<T> {\
			static void save(const T& value, OutStream& stream) {\
				stream.data = value;\
			}\
		};

		template<class T>
		struct Saver<T*> {
			static void save(const T* value, OutStream& stream) {
				if (value == nullptr) {
					stream.data = {};
				}
				else {
					stream.data = json::object();
					stream.data["meta"] = "ref";
					stream.data["val"] = reinterpret_cast<uint64_t>(value);
				}
			}
		};
		template<class T>
		struct Saver<Box<T>> {
			static void save(const Box<T>& value, OutStream& stream) {
				if (value == nullptr) {
					stream.data = {};
				}
				else if (stream.visited(value.get())) {
					throw std::runtime_error("Multiple strong reference to same object");
				}
				else {
					OutStream s(stream);
					value->serialize(s);
					stream.data = json::object();
					stream.data["meta"] = "val";
					stream.data["addr"] = reinterpret_cast<uint64_t>(value.get());
					stream.data["val"] = s.data;
					stream.addVisited(value.get());
				}
			}
		};
		struct InStream {
			struct State {
				std::unordered_map<uint64_t, Trait*> _ptrs;
			};
			const json& data;
			State* state;
			InStream(const InStream& rhs) :data(rhs.data), state(rhs.state) {}
			InStream(const json& data, State* state = nullptr) :data(data) {
				if (!state) {
					_stateOwner = std::make_unique<State>();
					this->state = _stateOwner.get();					
				}
				else {
					this->state = state;
				}
			}
			bool has(uint64_t i) {
				return state->_ptrs.find(i) != state->_ptrs.end();
			}
			Trait* get(uint64_t i) {
				return state->_ptrs.at(i);
			}
			void add(uint64_t i, Trait* t) {
				state->_ptrs[i] = t;
			}
		private:

			std::unique_ptr<State> _stateOwner;
		};
		template<typename Derived, typename Base, typename Del>
		std::unique_ptr<Derived, Del>
			static_unique_ptr_cast(std::unique_ptr<Base, Del>&& p)
		{
			auto d = static_cast<Derived*>(p.release());
			return std::unique_ptr<Derived, Del>(d, std::move(p.get_deleter()));
		}
		template<class T>
		struct Loader {
			static void load(T& value, InStream& stream) {
				TypeInfo* type = T::type();
				auto ty2 = stream.data.at("type").get<std::string>();
				if (ty2 != type->name()) {
					throw std::runtime_error(fmt::format("Excepted type {} but froud {}", type->name(), ty2));
				}
				InStream s(stream.data.at("val"), stream.state);
				InStreamVisitor visitor(s);
				visit(value, visitor);
			}
		};
		template<class T>
		struct Loader<Box<T>> {
			static void load(Box<T>& value, InStream& stream) {
				if (stream.data.is_null()) {
					value = nullptr; return;
				}
				auto meta = stream.data.at("meta").get<std::string>();
				if (meta == "ref") {
					throw std::runtime_error(fmt::format("multiple strong ref !!"));
				}
				else if (meta == "val") {
					auto val = stream.data.at("val");
					auto type = val.at("type");
					TypeInfo* info = getTypeByName(type.get<std::string>());
					value = std::move(static_unique_ptr_cast<T>(info->ctor()));
					InStream in(val, stream.state);
					info->loader(*value, in);
					auto addr = stream.data.at("addr").get<uint64_t>();
					CHECK(in.state == stream.state);
					stream.add(addr, value.get());
				}
				else {
					throw std::runtime_error(fmt::format("Unrecognized meta info: {}", meta));
				}
			}
		};
		template<class T>
		struct Loader<T*> {
			static void load(T*& value, InStream& stream) {
				if (stream.data.is_null()) {
					value = nullptr; return;
				}
				auto meta = stream.data.at("meta").get<std::string>();
				if (meta == "ref") {
					auto val = stream.data.at("val").get<uint64_t>();
					if (stream.has(val)) {
						value = static_cast<T*>(stream.get(val));
					}
					else {
						throw std::runtime_error(fmt::format("ref {} has not been loaded", val));
					}
				}
				else if (meta == "val") {
					throw std::runtime_error(fmt::format("weak ref !!"));
				}
				else {
					throw std::runtime_error(fmt::format("Unrecognized meta info: {}", meta));
				}
			}
		};
#define MYK_LOAD_TRIVIAL(T) \
		template<>\
		struct Loader<T> {\
			static void load(T& value, InStream& stream) {\
				value = stream.data.get<T>();\
			}\
		}
#define MYK_SAVE_LOAD_TRVIAL(T) MYK_LOAD_TRIVIAL(T);MYK_SAVE_TRIVIAL(T)
		MYK_SAVE_LOAD_TRVIAL(int);
		MYK_SAVE_LOAD_TRVIAL(Float);
		MYK_SAVE_LOAD_TRVIAL(Vec3f);
		MYK_SAVE_LOAD_TRVIAL(std::string);

		struct InStreamVisitor {
			int index = 0;
			InStream stream;
			InStreamVisitor(const InStream& stream) :stream(stream) {}
			template<class T>
			void visit(T& value, const char* name) {
				InStream stream(stream.data.at(index++), this->stream.state);
				Loader<T>::load(value, stream);
			}
		};


		struct TypeInfo {
			const char* _name;
			using Constructor = std::function<Box<Trait>()>;
			using Loader = std::function<void(Trait&, InStream&)>;
			using Saver = std::function<void(const Trait&, OutStream&)>;
			Constructor ctor;
			Loader loader;
			Saver saver;
			const char* name()const { return _name; }
			bool isOfName(const char* name) {
				return std::strcmp(name, _name) == 0;
			}
		};
		template<class T>
		TypeInfo* GetTypeInfo(const char* name) {
			static std::once_flag flag;
			static TypeInfo* info;
			std::call_once(flag, [&]() {
				info = new TypeInfo();
				info->_name = name;
				info->saver = [=](const Trait& value, OutStream& stream) {
					Saver<T>::save(static_cast<const T&>(value), stream);
				};
				info->ctor = [=]()->Box<Trait> {
					return make_box<T>();
				};
				info->loader = [=](Trait& value, InStream& stream) {
					Loader<T>::load(static_cast<T&>(value), stream);
				};
			});
			return info;
		}
		struct Trait {
			virtual TypeInfo* typeInfo() const = 0;
			virtual void serialize(OutStream& stream)const {
				typeInfo()->saver(*this, stream);
			}
			virtual ~Trait() = default;
		};
		template<int>
		struct ID {};
		template<class T>
		struct MetaInfo {

		};
		template<class T, class Visitor>
		void visit(T& x, Visitor& v) {
			using _T = std::decay_t<T>;
			MetaInfo<_T>::accept(x, v);
		}
		static std::set<TypeInfo*> _registerdTypes;
		static std::unordered_map<std::string, TypeInfo*> _registerdTypeMap;
		template<class T>
		inline void registerType() {
			auto t = T::type();
			_registerdTypes.insert(t);
			_registerdTypeMap[t->name()] = t;
		}
		inline TypeInfo* getTypeByName(const std::string& name) {
			return _registerdTypeMap.at(name);
		}
		TypeInfo* typeof(Trait* trait) {
			if (!trait)return nullptr;
			return trait->typeInfo();
		}
	}
	using Trait = Reflection::Trait;
#define MYK_AUTO_REGSITER_TYPE(Type) struct RegisterType##Type{using Self = RegisterType##Type;\
		Self(){Miyuki::Reflection::registerType<Type>();}\
	}; static RegisterType##Type regTypeInstance##Type;
#define MYK_BEGIN_REFL(Type) MYK_AUTO_REGSITER_TYPE(Type)template<>struct Miyuki::Reflection::MetaInfo<Type> { enum {__idx = __COUNTER__}; using __Self = Type;\
						template<int i>using UID = Miyuki::Reflection::ID<i>;static constexpr char * TypeName = #Type;

#define MYK_ATTR(name)  enum {__attr_index_##name = __COUNTER__ - __idx - 1 };\
						static auto& getAttribute(__Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto& getAttribute(const __Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto getAttributeName(UID<__attr_index_##name>) { return #name; }
#define MYK_END_REFL  enum{__attr_count =  __COUNTER__ - __idx - 1 };\
					template<class SelfT, class Visitor>\
					static void accept(SelfT& self,Visitor& visitor){ \
						__AcceptHelper<SelfT, Visitor, __attr_count - 1>::accept(self, visitor);\
					}\
					template<class SelfT, class Visitor, int i>\
					struct __AcceptHelper {\
						using Meta = Miyuki::Reflection::MetaInfo<__Self>;\
						static void accept(SelfT& self, Visitor& visitor) {\
							visitor.visit(Meta::getAttribute(self, UID<Meta::__attr_count - i - 1>()), Meta::getAttributeName(UID<Meta::__attr_count - i - 1>()));\
							__AcceptHelper<SelfT, Visitor, i - 1>::accept(self, visitor);\
						}\
					};\
					template<class SelfT, class Visitor>\
					struct __AcceptHelper<SelfT, Visitor, 0> {\
						using Meta = Miyuki::Reflection::MetaInfo<__Self>;\
						static void accept(SelfT& self, Visitor& visitor) {\
							visitor.visit(Meta::getAttribute(self, UID<Meta::__attr_count - 1>()), Meta::getAttributeName(UID<Meta::__attr_count - 1>()));\
						}\
					};};
#define MYK_IMPL(Type) using __Self = Type;\
		static Miyuki::Reflection::TypeInfo* type(){return Miyuki::Reflection::GetTypeInfo<Type>(#Type);}\
		virtual Miyuki::Reflection::TypeInfo* typeInfo() const override{\
			return __Self::type();\
		}

	
}

struct Foo : Miyuki::Trait {
	MYK_IMPL(Foo);
	int a;
	int b;
	Foo(int a, int b) :a(a), b(b) {}
	Foo() {}
};

MYK_BEGIN_REFL(Foo)
MYK_ATTR(a)
MYK_ATTR(b)
MYK_END_REFL

struct Bar : Miyuki::Trait {
	MYK_IMPL(Bar);
	int a;
	int b;
	Miyuki::Reflection::Box<Foo> foo;
	Foo* p;
};

MYK_BEGIN_REFL(Bar)
MYK_ATTR(a)
MYK_ATTR(b)
MYK_ATTR(foo)
MYK_ATTR(p)
MYK_END_REFL



int main(int argc, char** argv) {
	using namespace Miyuki::Reflection;
	/*Miyuki::GUI::MainWindow window(argc, argv);
	window.show();*/
	try {
		Bar bar;
		bar.a = 0;
		bar.b = 2;
		bar.foo = nullptr;// make_box<Foo>(1, 2);
		bar.p = bar.foo.get();
		OutStream stream;
		bar.serialize(stream);
		std::cout << stream.data.dump(2) << std::endl;
		Bar barrrr;
		InStream in(stream.data);
		Loader<Bar>::load(barrrr, in);
		OutStream s2;
		barrrr.serialize(s2);
		std::cout << s2.data.dump(2) << std::endl;
		std::cout << typeof(&bar)->name() << std::endl;
	}
	catch (json::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}