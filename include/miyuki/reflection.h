#ifndef MIYUKI_REFLECTION_H
#define MIYUKI_REFLECTION_H

#include <miyuki.h>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/algorithm/string/replace.hpp>


#define MYK_REFL_NIL (__nil)
#define _MYK_REFL_NIL __nil
namespace Miyuki {
	namespace Reflection {
		struct Nil {};
		template<class Visitor>
		void accept(const Nil&, Visitor) {}
		template<class Visitor>
		void accept(Nil&, Visitor) {}
		struct Reflective;
		struct Deleter {
			std::function<void(Reflective*)> deleter;
			Deleter() {}
			Deleter(std::function<void(Reflective*)> f) :deleter(std::move(f)) {}
			void operator()(Reflective* t) {
				deleter(t);
			}
		};
		template<class T = Reflective>
		using Box = std::unique_ptr<T, Deleter>;

		template<class T, class... Args>
		Box<T> make_box(Args... args) {
			return Box<T>(new T(args...), Deleter([](Reflective* p) {delete p; }));
		}

		class OutObjectStream {
			json data;
			struct State {
				std::set<const Reflective*> visited;
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
			void write(std::nullptr_t) {
				data = {};
			}
			void append(const OutObjectStream& stream) {
				data.push_back(stream.data);
			}
			void append(std::nullptr_t) {
				data.push_back(json{});
			}
			bool hasSerialized(const Reflective* object)const {
				return state->visited.find(object) != state->visited.end();
			}
			void addSerialized(const Reflective* object) {
				state->visited.insert(object);
			}
			template<class T>
			void write(const T& value) {
				data = value;
			}
			
			const json& toJson()const { return data; }
		};

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

		template<class T>
		std::enable_if_t<std::is_base_of_v<Reflective, T>, void> save(const T& value, OutObjectStream& stream) {
			auto sub = stream.sub();
			OutStreamVisitor visitor(sub);
			Miyuki::Reflection::visit(value, visitor);

			stream.write("type", T::type()->name());
			stream.write("val", sub);
		}


		template<class T>
		std::enable_if_t<std::is_base_of_v<Reflective, T>, void> save(const T* value, OutObjectStream& stream) {
			if (value == nullptr) {
				stream.write(nullptr);
			}
			else {
				stream.write("meta", "ref");
				stream.write("val", reinterpret_cast<uint64_t>(value));
			}
		}

		template<class T>
		std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
			save(const Box<T>& value, OutObjectStream& stream) {
			if (value == nullptr) {
				stream.write(nullptr);
			}
			else if (stream.hasSerialized(value.get())) {
				throw std::runtime_error("Multiple strong reference to same object");
			}
			else {
				auto s = stream.sub();
				value->serialize(s);
				stream.write("meta", "val");
				stream.write("addr", reinterpret_cast<uint64_t>(dynamic_cast<Reflective*>(value.get())));
				stream.write("val", s);
				stream.addSerialized(value.get());
			}
		}

		class InObjectStream {
			friend class Runtime;
			const json& data;
			struct State {
				std::unordered_map<size_t, Reflective*> map;
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
			bool has(size_t addr) {
				return state->map.find(addr) != state->map.end();
			}
			Reflective* fetchByAddr(size_t addr) {
				return state->map.at(addr);
			}
			void add(size_t addr, Reflective* v) {
				state->map[addr] = v;
			}
			size_t size() { return data.size(); }
		};
		template<typename Derived, typename Base, typename Del>
		inline std::unique_ptr<Derived, Del>
			static_unique_ptr_cast(std::unique_ptr<Base, Del>&& p)
		{
			auto d = dynamic_cast<Derived*>(p.release());
			return std::unique_ptr<Derived, Del>(d, std::move(p.get_deleter()));
		}

		template<typename Derived, typename Base>
		inline Box<Derived> downcast(Box<Base>&& p) {
			return static_unique_ptr_cast(p);
		}

		template<class T>
		inline std::enable_if_t<std::is_base_of_v<Reflective, T>, void> load(T& value, InObjectStream& stream) {
			TypeInfo* type = T::type();
			auto& data = stream.getJson();
			auto ty2 = data.at("type").get<std::string>();
			if (ty2 != type->name()) {
				throw std::runtime_error(fmt::format("Excepted type {} but froud {}", type->name(), ty2));
			}
			auto s = stream.sub("val");
			InStreamVisitor visitor(s);
			visit(value, visitor);
		}



		template<class T>
		inline std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
			load(Box<T>& value, InObjectStream& stream) {
			auto& data = stream.getJson();
			if (data.is_null()) {
				value = nullptr; return;
			}			
			auto meta = data.at("meta").get<std::string>();
			if (meta == "ref") {
				throw std::runtime_error(fmt::format("multiple strong ref !!"));
			}
			else if (meta == "val") {
				auto val = data.at("val");
				auto type = val.at("type");
				TypeInfo* info = getTypeByName(type.get<std::string>());
				auto addr = data.at("addr").get<uint64_t>();
				value = std::move(static_unique_ptr_cast<T>(info->ctor()));
				stream.add(addr, value.get());
				auto in = stream.sub("val");
				info->loader(*value, in);
						
			}
			else {
				throw std::runtime_error(fmt::format("Unrecognized meta info: {}", meta));
			}
		}

		template<class T>
		inline std::enable_if_t<std::is_base_of_v<Reflective, T>, void> load(T*& value, InObjectStream& stream) {
			auto& data = stream.getJson();
			if (data.is_null()) {
				value = nullptr; return;
			}
			auto meta = data.at("meta").get<std::string>();
			if (meta == "ref") {
				auto val = data.at("val").get<uint64_t>();
				if (stream.has(val)) {
					value = dynamic_cast<T*>(stream.fetchByAddr(val));
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
		template<class T>
		inline void save(const std::vector<T>& vec, OutObjectStream& stream) {
			for (const auto& item : vec) {
				auto sub = stream.sub();
				save(item, sub);
				stream.append(sub);
			}
		}
		template<class T>
		inline void load(std::vector<T>& vec, InObjectStream& stream) {
			for (auto i = 0; i < stream.size(); i++) {
				auto sub = stream.sub(i);
				T value;
				load(value, sub);
				vec.emplace_back(std::move(value));
			}
		}

		template<class K, class V>
		inline void save(const std::unordered_map<K, V>& map, OutObjectStream& stream) {
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
		inline void load(std::unordered_map<K, V>& map, InObjectStream& stream) {
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
		inline void save(const std::map<K, V>& map, OutObjectStream& stream) {
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
		inline void load(std::map<K, V>& map, InObjectStream& stream) {
			for (auto i = 0; i < stream.size(); i++) {
				auto pair = stream.sub(i);
				K key;
				V val;
				load(key, pair.sub("key"));
				load(val, pair.sub("val"));
				map[key] = val;
			}
		}

#define MYK_SAVE_TRIVIAL(type)\
		inline void save(const type& value, Miyuki::Reflection::OutObjectStream& stream) {\
			stream.write(value);\
		}
#define MYK_LOAD_TRIVIAL(type)\
		inline void load(type& value, Miyuki::Reflection::InObjectStream& stream) {\
			value = stream.getJson().get<type>();\
		}

#define MYK_SAVE_LOAD_TRIVIAL(type) 	MYK_SAVE_TRIVIAL(type)MYK_LOAD_TRIVIAL(type)
		MYK_SAVE_LOAD_TRIVIAL(bool);
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

		namespace detail {
			template<class T>
			std::enable_if_t<!std::is_pointer_v<T>, void> ZeroInit(T& value) { value = T(); }
			
			template<class T>
			std::enable_if_t<std::is_pointer_v<T>, void> ZeroInit(T& value) { value = nullptr; }
			
		}
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
				else {
					detail::ZeroInit(value);
				}
			}
		};

		enum TypeKind {
			EInterface,
			EAbstract,
			EImplementation
		};
		struct TypeInfo {
			const char* _name;
			using Constructor = std::function<Box<Reflective>(void)>;
			using Loader = std::function<void(Reflective&, InObjectStream&)>;
			using Saver = std::function<void(const Reflective&, OutObjectStream&)>;
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
				info->saver = [=](const Reflective& value, OutObjectStream& stream) {
					save(dynamic_cast<const T&>(value), stream);
				};
				info->ctor = [=]()->Box<Reflective> {
					return make_box<T>();
				};
				info->loader = [=](Reflective& value, InObjectStream& stream) {
					load(dynamic_cast<T&>(value), stream);
				};
			});
			return info;
		}
		template<class T>
		TypeInfo* GetAbstractTypeInfo(const char* name) {
			static std::once_flag flag;
			static TypeInfo* info;
			std::call_once(flag, [&]() {
				info = new TypeInfo();
				info->_name = name;
				info->ctor = [=]()->Box<Reflective> {
					throw std::runtime_error("Attempt to create abstract class");
					return nullptr;
				};
			});
			return info;
		}
		struct __Injector {
			template<class F>
			__Injector(F&& f) { std::move(f)(); }
		};
		template<class T>
		struct GetSelf {};

		template<class R, class T, class... Args>
		struct GetSelf<R(T::*)(Args...)> {
			using type = T;
		};
#define MYK_TYPE_KIND(kind) static const Miyuki::Reflection::TypeKind typeKind = kind;
#define MYK_GET_SELF void __get_self_helper(); using Self = Miyuki::Reflection::GetSelf<decltype(&__get_self_helper)>::type;
#define MYK_INTERFACE(Interface) static const std::string& interfaceInfo(){\
									static std::string s = "Interface." #Interface;\
									return s; \
								}\
								MYK_TYPE_KIND(Miyuki::Reflection::EInterface)
								

#define _MYK_EXTENDS(Interface, Super) \
	static Miyuki::Reflection::__Injector \
		BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(Extend_ ## Interface ##_ ##injector_, __COUNTER__),_), __LINE__)\
		([]() {\
			Miyuki::Reflection::extendInterface<Interface, Super>(); \
	});
#define _MYK_EXTENDS_SEQ_MACRO(r, data, elem) _MYK_EXTENDS(data, elem)
#define MYK_EXTENDS(Interface, Supers) BOOST_PP_SEQ_FOR_EACH(_MYK_EXTENDS_SEQ_MACRO, Interface, Supers)
		struct ComponentVisitor;
		struct Reflective {
			MYK_INTERFACE(Reflective);
			virtual TypeInfo* typeInfo() const = 0;
			virtual void serialize(OutObjectStream& stream)const {
				typeInfo()->saver(*this, stream);
			}
			virtual void deserialize(InObjectStream& stream) {
				typeInfo()->loader(*this, stream);
			}
			virtual ~Reflective() = default;
			inline void accept(ComponentVisitor& visitor);
		protected:
			static Nil _MYK_REFL_NIL;
		};
		struct ComponentVisitor {
			std::unordered_map<TypeInfo*, std::function<void(Reflective*)>>_map;
		public:
			void visit(Reflective* trait) {
				if (!trait)return;
				_map.at(trait->typeInfo())(trait);
			}
			template<class T>
			void visit(Box<T>& trait) {
				visit(trait.get());
			}
			template<class T>
			void visit(const std::function<void(T*)>& f) {
				_map[T::type()] = [=](Reflective* p) {
					f(dynamic_cast<T*>(p));
				};
			}
		};
		namespace detail {
			template<class T = Reflective>
			struct Match {
				T* value;
				bool matched = false;
				Match(T* value) :value(value) {}
				template<class U, class Function>
				Match& with(Function func) {
					if (matched)return *this;
					if (value && typeof(value) == typeof<U>()) {
						auto f = std::move(func);
						f(dynamic_cast<U*>(value));
						matched = true;
					}
					return *this;
				}
			};
		}
		template<class T = Reflective>
		detail::Match<T> match(T * trait) {
			return detail::Match<T>(trait);
		};
		inline void Reflective::accept(ComponentVisitor& visitor) {
			visitor.visit(this);
		}
		template<int>
		struct ID {};

		template<class T, class Visitor>
		void visit(T& x, Visitor& v) {
			using _T = std::decay_t<T>;
			accept(x, v);
		}
		namespace detail {
			struct TypeInfoCompare {
				bool operator ()(const TypeInfo* a, const TypeInfo* b)const {
					return std::less<std::string>()(a->name(), b->name());
				}
			};

			using ImplSet = std::set<TypeInfo*, TypeInfoCompare>;
			struct InterfaceTree {
				std::string name;
				std::set<InterfaceTree*> derived;
				ImplSet directImpl;
				template<class Impl>
				void addDirectImpl() {
					directImpl.insert(Impl::type());
				}
				ImplSet getAllImpls()const {
					ImplSet set = directImpl;
					for (auto d : derived) {
						auto s = d->getAllImpls();
						for (auto i : s) {
							set.insert(i);
						}
					}
					return set;
				}
			};
			struct Types {
				std::set<TypeInfo*> _registerdTypes;
				std::unordered_map<std::string, TypeInfo*> _registerdTypeMap;
				std::unordered_map<std::string, std::unique_ptr<InterfaceTree>> _tree;
				static Types* all;
				static std::once_flag flag;
				static Types& get() {
					std::call_once(flag, [&]() {all = new Types(); });
					return *all;
				}
				template<class T>
				void addInterface(){
					_tree[T::interfaceInfo()] = std::make_unique<InterfaceTree>();
					_tree[T::interfaceInfo()]->name = T::interfaceInfo();
				}
				template<class Derived, class Base>
				void extendInterface() {
					if (_tree.find(Base::interfaceInfo()) == _tree.end())
						addInterface<Base>();
					if (_tree.find(Derived::interfaceInfo()) == _tree.end())
						addInterface<Derived>();
					auto& base = _tree.at(Base::interfaceInfo());
					auto& derived = _tree.at(Derived::interfaceInfo());
					base->derived.insert(derived.get());

				}
				template<class Impl, class Interface>
				inline void addImpl() {
					auto t = Impl::type();
					auto inter = Interface::interfaceInfo();
					if (_tree.find(inter) == _tree.end())
						addInterface<Interface>();
					_tree.at(inter)->addDirectImpl<Impl>();
				}
			};

		}
		template<class T>
		inline void registerType() {
			auto t = T::type();
			detail::Types::get()._registerdTypes.insert(t);
			detail::Types::get()._registerdTypeMap[t->name()] = t;
		}

		template<class Impl, class Interface>
		inline void registerImplementation() {
			detail::Types::get().addImpl<Impl, Interface>();			
		}

		template<class Derived, class Base>
		inline void extendInterface() {
			detail::Types::get().extendInterface<Derived, Base>();
		}


		template<class Interface>
		inline detail::ImplSet getImplementations() {
			auto inter = Interface::interfaceInfo();
			const auto& _tree = detail::Types::get()._tree;
			const auto& node = _tree.at(inter);
			return node->getAllImpls();
		}

		inline Box<Reflective> createComponent(const std::string& name) {
			auto t = detail::Types::get()._registerdTypeMap.at(name);
			return t->ctor();
		}
		inline TypeInfo* getTypeByName(const std::string& name) {
			return detail::Types::get()._registerdTypeMap.at(name);
		}
		inline TypeInfo* typeof(Reflective* trait) {
			if (!trait)return nullptr;
			return trait->typeInfo();
		}
		template<class T>
		inline TypeInfo* typeof() {
			return T::type();
		}

	}

	using Reflective = Reflection::Reflective;
	using Reflection::Box;
	template<class T>
	using Arc = std::shared_ptr<T>;

#define MYK_ABSTRACT(Abstract) MYK_GET_SELF struct Meta; static const std::string& interfaceInfo(){\
									static std::string s = "Abstract." #Abstract;\
									return s; \
								}\
								MYK_TYPE_KIND(Miyuki::Reflection::EAbstract)\
								static inline Miyuki::Reflection::TypeInfo* type() {\
									return Miyuki::Reflection::GetAbstractTypeInfo<Self>("Abstract." #Abstract);\
								}\
								Miyuki::Reflection::TypeInfo* typeInfo() const{\
									return Self::type();\
								}\
								_MYK_GEN_BASE_INFO()
#define _MYK_GEN_BASE_INFO() \
	template<size_t> \
	struct GetBaseType { \
		static const int has = false;\
	};

#define MYK_INHERITS(base) static_assert(base::typeKind != Miyuki::Reflection::EInterface, "You cannot only use MYK_BASE on Interface");\
						template<>\
						struct GetBaseType<0>{\
							static const int has = true;\
							using type = base;\
						};\
						MYK_TYPE_KIND(Miyuki::Reflection::EImplementation)
						
#define MYK_META MYK_GET_SELF struct Meta;\
		static inline Miyuki::Reflection::TypeInfo* type();\
		Miyuki::Reflection::TypeInfo* typeInfo() const{\
			return Self::type();\
		}\
		_MYK_GEN_BASE_INFO()
#define MYK_CLASS(type) MYK_META

#define MYK_AUTO_REGSITER_TYPE(Type, Alias)\
		struct Injector_##Type{\
			Injector_##Type(){\
				Miyuki::Reflection::registerType<Type>();\
			}\
		};static Injector_##Type _injector_##Type;\
inline Miyuki::Reflection::TypeInfo* Type::type(){return Miyuki::Reflection::GetTypeInfo<Type>(Alias);}

#define MYK_INJECT_INTERFACE_IMPL(Type, Interface) \
	static Miyuki::Reflection::__Injector \
		BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(Impl_ ## Type ##_ ##injector_, __COUNTER__),_), __LINE__)\
		([]() {\
			Miyuki::Reflection::registerImplementation<Type, Interface>(); \
	});
#define _MYK_IMPL_SEQ_MACRO(r, data, elem) MYK_INJECT_INTERFACE_IMPL(data, elem)

#define MYK_IMPL(Type, Interfaces, Alias) MYK_AUTO_REGSITER_TYPE(Type, Alias)\
								BOOST_PP_SEQ_FOR_EACH(_MYK_IMPL_SEQ_MACRO, Type, Interfaces)

#define MYK_BEGIN_REFL(Type) struct Type::Meta {\
						 enum {__idx = __COUNTER__}; using __Self = Type;\
						template<int i>using UID = Miyuki::Reflection::ID<i>;static constexpr char * TypeName = #Type;

#define MYK_ATTR(name)  enum {__attr_index_##name = __COUNTER__ - __idx - 1 };\
						static auto& getAttribute(__Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto& getAttribute(const __Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto getAttributeName(UID<__attr_index_##name>) { return #name; }
#define MYK_END_REFL(Type)  enum{__attr_count =  __COUNTER__ - __idx - 1 };\
					template<class Visitor>\
					static void accept(const __Self& self,Visitor& visitor){ \
						if constexpr (__Self::GetBaseType<0>::has){\
							using base = typename __Self::GetBaseType<0>::type;\
							base::Meta::accept(static_cast<const base&>(self), visitor);\
						}\
						__AcceptHelper<const __Self, Visitor, __attr_count - 1>::accept(self, visitor);\
					}\
					template<class Visitor>\
					static void accept(__Self& self,Visitor& visitor){ \
						if constexpr (__Self::GetBaseType<0>::has){\
							using base = typename __Self::GetBaseType<0>::type;\
							base::Meta::accept(static_cast<base&>(self), visitor);\
						}\
						__AcceptHelper<__Self, Visitor, __attr_count - 1>::accept(self, visitor);\
					}\
					template<class SelfT, class Visitor, int i>\
					struct __AcceptHelper {\
						static void accept(SelfT& self, Visitor& visitor) {\
						if constexpr (!std::is_same_v<std::decay_t<decltype(Meta::getAttribute(self, UID<Meta::__attr_count - i - 1>()))>,\
								Miyuki::Reflection::Nil>)\
							visitor.visit(Meta::getAttribute(self, UID<Meta::__attr_count - i - 1>()), Meta::getAttributeName(UID<Meta::__attr_count - i - 1>()));\
							__AcceptHelper<SelfT, Visitor, i - 1>::accept(self, visitor);\
						}\
					};\
					template<class SelfT, class Visitor>\
					struct __AcceptHelper<SelfT, Visitor, 0> {\
					static void accept(SelfT& self, Visitor& visitor) {\
							if constexpr (!std::is_same_v<std::decay_t<decltype(Meta::getAttribute(self, UID<Meta::__attr_count - 1>()))>,\
								Miyuki::Reflection::Nil>)\
							visitor.visit(Meta::getAttribute(self, UID<Meta::__attr_count - 1>()), Meta::getAttributeName(UID<Meta::__attr_count - 1>()));\
						}\
					};};\
					template<class Visitor>\
					void accept(Type& self, Visitor vis){\
						Type::Meta::accept(self, vis);\
					}\
					template<class Visitor>\
					void accept(const Type& self, Visitor vis){\
						Type::Meta::accept(self, vis);\
					}

#define MYK_SEQ_MACRO(r, data, elem) MYK_ATTR(elem)
#define MYK_REFL(Type, Attributes) MYK_BEGIN_REFL(Type) BOOST_PP_SEQ_FOR_EACH(MYK_SEQ_MACRO, _, Attributes) MYK_END_REFL(Type)

}

#define MYK_REFL_IMPLEMENTATION Miyuki::Reflection::detail::Types* Miyuki::Reflection::detail::Types::all;\
std::once_flag  Miyuki::Reflection::detail::Types::flag;\
Miyuki::Reflection::Nil Miyuki::Reflection::Reflective::  _MYK_REFL_NIL;

#endif