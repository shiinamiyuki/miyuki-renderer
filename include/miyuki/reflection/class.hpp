#ifndef MIYUKI_REFLECTION_CLASS_HPP
#define MIYUKI_REFLECTION_CLASS_HPP
#include <miyuki.h>
#include <utils/noncopymovable.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>

namespace Miyuki {
	namespace Reflection {
		class Object;
		struct UUID : boost::uuids::uuid {
			UUID(const boost::uuids::uuid& id) :boost::uuids::uuid(id) {}
			UUID() :boost::uuids::uuid() {}
			std::string str()const {
				return boost::uuids::to_string(*this);
			}
			struct Hash
			{
				size_t operator () (const UUID& uid)const
				{
					return boost::hash<boost::uuids::uuid>()(uid);
				}
			};
		};
	
		struct Class : NonCopyMovable{
			using Constructor = std::function<Object* (const UUID&)>;
			struct {
				const Class* base = nullptr;
				Constructor ctor; 
				size_t size = 0;
				bool isFinal = false;
			} classInfo;
			const char* _name;
			const char* name() const{
				return _name;
			}
			Object* create(const UUID& id)const { return classInfo.ctor(id); }
		};
	}
}
template <>
struct fmt::formatter<Miyuki::Reflection::UUID> {
	using UUID = Miyuki::Reflection::UUID;
	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const UUID& d, FormatContext& ctx) {
		return format_to(ctx.out(), "{}", d.str());
	}
};
#endif