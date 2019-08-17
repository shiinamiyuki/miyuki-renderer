#ifndef MIYUKI_REFLECTION_H
#define MIYUKI_REFLECTION_H

#include <miyuki.h>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/algorithm/string/replace.hpp>


#define MYK_REFL_NIL (__nil)
#define _MYK_REFL_NIL __nil
#define MYK_NIL_BASE (Miyuki::Reflection::Nil)
namespace Miyuki {
	namespace Reflection {
		struct Nil {
			Nil() {}
			struct Meta {
				template<class T>
				static void accept(Nil&, T) {}
				template<class T>
				static void accept(const Nil&, T) {}
			};
		};
		template<class Visitor>
		void accept(const Nil&, Visitor) {}
		template<class Visitor>
		void accept(Nil&, Visitor) {}
		class Reflective;
		struct Deleter {
			std::function<void(Reflective*)> deleter;
			inline Deleter();
			Deleter(std::function<void(Reflective*)> f) :deleter(std::move(f)) {}
			void operator()(Reflective* t) {
				deleter(t);
			}
		};
		template<class T = Reflective>
		using Box = std::unique_ptr<T, Deleter>;
		template<class T>
		using Arc = std::shared_ptr<T>;
		template<class T>
		using Weak = std::weak_ptr<T>;
		template<class T, class... Args>
		Arc<T> makeArc(Args&& ... args) {
			return std::make_shared<T>(args...);
		}
		template<class T, class... Args>
		Box<T> makeBox(Args&& ... args) {
			return Box<T>(new T(args...), Deleter([](Reflective* p) {delete p; }));
		}

		class OutObjectStream {
			json data;
			struct State {
				std::set<const Reflective*> visited;
				json refObjects;
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
			void writeRef(size_t addr, const OutObjectStream& stream) {
				state->refObjects[std::to_string(addr)] = stream.data;
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

			json toJson()const {
				json j;
				j["ref"] = state->refObjects;
				j["val"] = data;
				return j;
			}
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
		std::enable_if_t<std::is_base_of_v<Reflective, T>, void> save(const Weak<T>& value, OutObjectStream& stream) {
			if (value.expired()) {
				stream.write(nullptr);
			}
			else {
				stream.write("meta", "ref");
				stream.write("val", reinterpret_cast<uint64_t>(value.lock().get()));
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
				auto addr = reinterpret_cast<uint64_t>(dynamic_cast<Reflective*>(value.get()));
				value->serialize(s);
				stream.write("meta", "ref");
				stream.write("addr", addr);
				s.write("meta", "box");
				stream.writeRef(addr, s);
				stream.addSerialized(value.get());
			}
		}
		template<class T>
		std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
			save(const Arc<T>& value, OutObjectStream& stream) {
			if (value == nullptr) {
				stream.write(nullptr);
			}
			else if (stream.hasSerialized(value.get())) {
				throw std::runtime_error("Multiple strong reference to same object");
			}
			else {
				auto s = stream.sub();
				auto addr = reinterpret_cast<uint64_t>(dynamic_cast<Reflective*>(value.get()));
				if (!stream.hasSerialized(value.get())) {
					value->serialize(s);
					stream.addSerialized(value.get());
				}
				stream.write("meta", "ref");
				stream.write("addr", addr);
				s.write("meta", "arc");
				stream.writeRef(addr, s);

			}
		}
		class InObjectStream {
			friend class Runtime;
			const json& data;
			struct State {
				std::unordered_map<size_t, Reflective*> map;
				std::unordered_map<size_t, Box<Reflective>> boxes;
				std::unordered_map<size_t, Arc<Reflective>> arcs;
				std::unordered_map<size_t, bool> s_arcs;
				const json& root;
				State(const json& root) :root(root) {

				}
			};
			std::shared_ptr<State> state;
			inline void initializeRefObjects();
		public:
			std::pair<Box<Reflective>, InObjectStream> getBox(size_t addr) {
				auto box = std::move(state->boxes.at(addr));
				state->boxes.erase(addr);
				auto in = InObjectStream(state->root.at("ref").at(std::to_string(addr)), state);
				return std::make_pair(std::move(box), std::move(in));
			}
			bool hasDeserializedArc(size_t i) {
				return state->s_arcs.find(i) != state->s_arcs.end();
			}
			void addDersializedArc(size_t i) {
				state->s_arcs[i] = true;
			}
			std::pair<Arc<Reflective>, InObjectStream> getArc(size_t addr) {
				auto arc = state->arcs.at(addr);
				auto in = InObjectStream(state->root.at("ref").at(std::to_string(addr)), state);
				return std::make_pair(arc, std::move(in));
			}
			InObjectStream(const json& data) :data(data.at("val")) {
				state = std::make_shared<State>(data);
				initializeRefObjects();
			}
			InObjectStream(const json& data, std::shared_ptr<State> s) :data(data) {
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
				auto addr = data.at("addr").get<uint64_t>();
				auto [box, in] = std::move(stream.getBox(addr));
				value = std::move(static_unique_ptr_cast<T>(std::move(box)));
				stream.add(addr, value.get());
				value->typeInfo()->loader(*value, in);

			}
			else {
				throw std::runtime_error(fmt::format("Unrecognized meta info: {}", meta));
			}
		}
		template<class T>
		inline std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
			load(Arc<T>& value, InObjectStream& stream) {
			auto& data = stream.getJson();
			if (data.is_null()) {
				value = nullptr; return;
			}
			auto meta = data.at("meta").get<std::string>();
			if (meta == "ref") {
				auto addr = data.at("addr").get<uint64_t>();
				auto [arc, in] = std::move(stream.getArc(addr));
				value = std::dynamic_pointer_cast<T>(arc);
				if (stream.hasDeserializedArc(addr)) {

				}
				else {
					stream.add(addr, value.get());
					value->typeInfo()->loader(*value, in);
					stream.addDersializedArc(addr);
				}
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
		inline std::enable_if_t<std::is_base_of_v<Reflective, T>, void> load(Weak<T>& value, InObjectStream& stream) {
			auto& data = stream.getJson();
			if (data.is_null()) {
				value.reset();
			}
			auto meta = data.at("meta").get<std::string>();
			if (meta == "ref") {
				auto val = data.at("val").get<uint64_t>();
				if (stream.has(val)) {
					auto [arc, in] = stream.getArc(val);
					value = std::dynamic_pointer_cast<T>(arc);
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
			struct Compare {
				bool operator ()(const TypeInfo* a, const TypeInfo* b)const {
					return std::less<std::string>()(a->name(), b->name());
				}
			};

			using DerivedSet = std::set<const TypeInfo*, Compare>;
			const char* _name;
			using Constructor = std::function<Reflective* (void)>;
			using Loader = std::function<void(Reflective&, InObjectStream&)>;
			using Saver = std::function<void(const Reflective&, OutObjectStream&)>;
			Constructor ctor;
			Loader loader;
			Saver saver;
			TypeKind kind;
			DerivedSet derived;
			const char* name()const { return _name; }
			bool isOfName(const char* name)const {
				return std::strcmp(name, _name) == 0;
			}
			void addDerived(const TypeInfo* info) {
				derived.insert(info);
			}
			DerivedSet getAllDerived()const {
				DerivedSet s = derived;
				for (auto i : derived) {
					auto tmp = i->getAllDerived();
					for (auto j : tmp) {
						s.insert(j);
					}
				}
				return s;
			}
		};
		template<class Interface>
		TypeInfo::DerivedSet getImplementations() {
			static_assert(Interface::typeKind == EInterface || Interface::typeKind == EAbstract, "Interface must bt abstract");
			TypeInfo::DerivedSet set = Interface::type()->getAllDerived();
			TypeInfo::DerivedSet result;
			for (auto i : set) {
				if (i->kind == EImplementation)
					result.insert(i);
			}
			return result;

		}
		namespace detail {
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
					info->ctor = [=]()->Reflective * {
						return new T();
					};
					info->loader = [=](Reflective& value, InObjectStream& stream) {
						load(dynamic_cast<T&>(value), stream);
					};
					info->kind = T::typeKind;
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
					info->ctor = [=]()->Reflective * {
						throw std::runtime_error("Attempt to create abstract class");
						return nullptr;
					};
					info->kind = T::typeKind;
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
		}
#define MYK_TYPE_KIND(kind) static const Miyuki::Reflection::TypeKind typeKind = kind;
#define MYK_GET_SELF void __get_self_helper(); using Self = Miyuki::Reflection::detail::GetSelf<decltype(&__get_self_helper)>::type;
#define MYK_INTERFACE(Interface) MYK_GET_SELF struct Meta;static inline Miyuki::Reflection::TypeInfo* type() {\
									return Miyuki::Reflection::detail::GetAbstractTypeInfo<Self>("Interface." #Interface);\
								}\
								MYK_TYPE_KIND(Miyuki::Reflection::EInterface)\
								static Miyuki::Reflection::Nil _MYK_REFL_NIL;

#define _MYK_BEGIN_EXTEND() enum{__base__idx = __COUNTER__ - 1;}

#define _MYK_EXTENDS(Base, Super) \
	static Miyuki::Reflection::__Injector \
		BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(Extend_ ## Interface ##_ ##injector_, __COUNTER__),_), __LINE__)\
		([]() {\
			Miyuki::Reflection::details::Types::get().extend<Base, Super>(); \
	});



#define _MYK_EXTENDS_SEQ_MACRO(r, data, elem) _MYK_EXTENDS(data, elem)
#define MYK_EXTENDS(Base, Supers) BOOST_PP_SEQ_FOR_EACH(_MYK_EXTENDS_SEQ_MACRO, Base, Supers)

		struct ReflectionVisitor;

		template<int>
		struct ID {};

		template<class T, class Visitor>
		void visit(T& x, Visitor& v) {
			using _T = std::decay_t<T>;
			accept(x, v);
		}
		namespace detail {

			struct Types {
				std::set<TypeInfo*> _registerdTypes;
				std::unordered_map<std::string, TypeInfo*> _registerdTypeMap;
				static Types* all;
				static std::once_flag flag;
				static Types& get() {
					std::call_once(flag, [&]() {all = new Types(); });
					return *all;
				}
				template<class Derived, class Base>
				std::enable_if_t<!std::is_same_v<Base, Nil>, void> extend() {
					TypeInfo* base = Base::type();
					TypeInfo* derived = Derived::type();
					base->addDerived(derived);
				}
				template<class Derived, class Base>
				std::enable_if_t<std::is_same_v<Base, Nil>, void> extend() {
				}
				template<class T>
				void registerType() {
					auto t = T::type();
					_registerdTypes.insert(t);
					_registerdTypeMap[t->name()] = t;
				}
			};

		}
#define MYK_ABSTRACT(Abstract) MYK_GET_SELF struct Meta; \
								MYK_TYPE_KIND(Miyuki::Reflection::EAbstract)\
								static inline Miyuki::Reflection::TypeInfo* type() {\
									return Miyuki::Reflection::detail::GetAbstractTypeInfo<Self>("Abstract." #Abstract);\
								}\
								Miyuki::Reflection::TypeInfo* typeInfo() const{\
									return Self::type();\
								}\
								static Miyuki::Reflection::Nil _MYK_REFL_NIL;

#define MYK_META MYK_GET_SELF struct Meta;\
		static inline Miyuki::Reflection::TypeInfo* type();\
		Miyuki::Reflection::TypeInfo* typeInfo() const{\
			return Self::type();\
		}

#define MYK_CLASS(type) MYK_META\
						MYK_TYPE_KIND(Miyuki::Reflection::EImplementation)


#define MYK_AUTO_REGSITER_TYPE(Type, Alias)\
		struct Injector_##Type{\
			Injector_##Type(){\
				Miyuki::Reflection::detail::Types::get().registerType<Type>();\
			}\
		};static Injector_##Type _injector_##Type;\
inline Miyuki::Reflection::TypeInfo* Type::type(){return Miyuki::Reflection::detail::GetTypeInfo<Type>(Alias);}

#define MYK_INJECT_INTERFACE_IMPL(Type, Interface) \
	static Miyuki::Reflection::detail::__Injector \
		BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(Impl_ ## Type ##_ ##injector_, __COUNTER__),_), __LINE__)\
		([]() {\
			Miyuki::Reflection::detail::Types::get().extend<Type, Interface>(); \
	});
#define _MYK_IMPL_SEQ_MACRO(r, data, elem) MYK_INJECT_INTERFACE_IMPL(data, elem)

#define MYK_IMPL(Type, Alias) MYK_AUTO_REGSITER_TYPE(Type, Alias)

#define MYK_BEGIN_REFL(Type) struct Type::Meta {\
						 enum {__idx = __COUNTER__}; using __Self = Type;\
						template<int i>using UID = Miyuki::Reflection::ID<i>;static constexpr char * TypeName = #Type;

#define MYK_ATTR(name)  enum {__attr_index_##name = __COUNTER__ - __idx - 1 };\
						static auto& getAttribute(__Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto& getAttribute(const __Self& self, UID<__attr_index_##name>){return self.name;} \
						static auto getAttributeName(UID<__attr_index_##name>) { return #name; }


#define _MYK_ACCEPT_BASE(r,data, Base) \
if constexpr (!std::is_same_v<std::decay_t<Base>,Miyuki::Reflection::Nil>)Base::Meta::accept(static_cast<Base&>(self), visitor);
#define _MYK_ACCEPT_BASES(Bases)  BOOST_PP_SEQ_FOR_EACH(_MYK_ACCEPT_BASE, _ , Bases)
#define _MYK_ACCEPT_BASE_CONST(r,data, Base) \
if constexpr (!std::is_same_v<std::decay_t<Base>,Miyuki::Reflection::Nil>)Base::Meta::accept(static_cast<const Base&>(self), visitor);
#define _MYK_ACCEPT_BASES_CONST(Bases)  BOOST_PP_SEQ_FOR_EACH(_MYK_ACCEPT_BASE_CONST, _ , Bases)
#define MYK_END_REFL(Type, Bases)  enum{__attr_count =  __COUNTER__ - __idx - 1 };\
					template<class Visitor>\
					static void accept(__Self& self,Visitor& visitor){ \
						_MYK_ACCEPT_BASES(Bases);\
						__AcceptHelper<__Self, Visitor, __attr_count - 1>::accept(self, visitor);\
					}\
					template<class Visitor>\
					static void accept(const __Self& self,Visitor& visitor){ \
						_MYK_ACCEPT_BASES_CONST(Bases);\
						__AcceptHelper<const __Self, Visitor, __attr_count - 1>::accept(self, visitor);\
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
#define MYK_REFL(Type, Supers, Attributes) MYK_BEGIN_REFL(Type) BOOST_PP_SEQ_FOR_EACH(MYK_SEQ_MACRO, _, Attributes) MYK_END_REFL(Type, Supers)\
											BOOST_PP_SEQ_FOR_EACH(_MYK_IMPL_SEQ_MACRO, Type, Supers )


		class Reflective {
		public:
			MYK_INTERFACE(Reflective);
			virtual TypeInfo* typeInfo() const = 0;
			virtual void serialize(OutObjectStream& stream)const {
				typeInfo()->saver(*this, stream);
			}
			virtual void deserialize(InObjectStream& stream) {
				typeInfo()->loader(*this, stream);
			}
			virtual ~Reflective() = default;
			inline void accept(ReflectionVisitor& visitor);
		};
		MYK_REFL(Reflective, MYK_NIL_BASE, MYK_REFL_NIL);
		struct ReflectionVisitor {
			std::unordered_map<TypeInfo*, std::function<void(Reflective*)>>_map;
			template<class F>
			struct FuncTraits {

			};
		public:
			void visit(Reflective* trait) {
				if (!trait)return;
				_map.at(trait->typeInfo())(trait);
			}
			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void> visit(Box<T>& p) {
				visit(p.get());
			}
			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void> visit(Arc<T>& p) {
				visit(p.get());
			}
			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void> visit(Weak<T>& p) {
				visit(p.lock());
			}
			template<class T>
			std::enable_if_t<std::is_base_of_v<Reflective, T>, void>
				whenVisit(const std::function<void(T*)>& f) {
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
		inline void Reflective::accept(ReflectionVisitor& visitor) {
			visitor.visit(this);
		}


		template<class Interface>
		Interface* cast(Reflective* ptr) {
			return dynamic_cast<Interface*>(ptr);
		}

		inline Box<Reflective> createByName(const std::string& name) {
			auto t = detail::Types::get()._registerdTypeMap.at(name);
			return Box<Reflective>(t->ctor(), Deleter());
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
		void InObjectStream::initializeRefObjects() {
			const auto& ref = state->root.at("ref");
			for (const auto& o : ref.items()) {
				const auto& k = o.key();
				const auto& v = o.value();
				auto type = getTypeByName(v.at("type").get<std::string>());
				auto object = type->ctor();
				size_t addr = std::stoull(k);
				state->map[addr] = object;
				if (v.at("meta") == "box") {
					state->boxes[addr] = Box<Reflective>(object);

				}
				else if (v.at("meta") == "arc") {
					state->arcs[addr] = Arc<Reflective>(object);
				}
				else {
					throw std::runtime_error("unknown meta");
				}
			}
		}
		Deleter::Deleter() :deleter([](Reflective* p) {delete p; }) {}
	}

	using Reflective = Reflection::Reflective;
	using Reflection::Box;
	using Reflection::Arc;
	using Reflection::Weak;
	using Reflection::makeBox;
	using Reflection::makeArc;

}
#define MYK_REFL_IMPLEMENTATION Miyuki::Reflection::detail::Types* Miyuki::Reflection::detail::Types::all;\
std::once_flag  Miyuki::Reflection::detail::Types::flag;\
Miyuki::Reflection::Nil Miyuki::Reflection::Reflective::  _MYK_REFL_NIL;

#endif