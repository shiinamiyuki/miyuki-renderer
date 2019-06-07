#ifndef MIYUKI_REFLECTION_REFLECTION_HPP
#define MIYUKI_REFLECTION_REFLECTION_HPP

#include "object.hpp"
#include "primitive.hpp"
#include "array.hpp"

#define MYK_PROPERTY(Type, Name)  \
	using Name##_Type = Miyuki::Reflection::PropertyT<Miyuki::Reflection::_ConvertToPrimitiveType<Type>::type>;\
	enum { _propertyIdx ## Name = __COUNTER__ - _propertyIdx - 1};\
	Name##_Type Name = decltype(Name)(#Name); \
	auto& getProperty(Miyuki::Reflection::UID<_propertyIdx  ## Name>){return Name;}\
	const auto& getProperty(Miyuki::Reflection::UID<_propertyIdx ## Name>)const{return Name;}\
	auto& getName(Miyuki::Reflection::UID<_propertyIdx ## Name>){return #Name;}

#define MYK_BEGIN_PROPERTY 
		
#define MYK_END_PROPERTY enum{_propertyCount =  __COUNTER__ - _propertyIdx - 1 }; \
	const std::vector<const Miyuki::Reflection::Property*> getProperties()const override{ \
		auto vec = std::move(BaseT::getProperties()); \
		Miyuki::Reflection::_GetPropertiesHelper<std::decay<decltype(*this)>::type, _propertyCount>::_GetProperties(*this, vec); \
		return std::move(vec);\
	}
	
#define MYK_CTOR(...)  void init(__VA_ARGS__ )
#define MYK_CLASS(Classname, Base) MYK_CLASS_TYPE_INFO(Classname, Base) \
									Classname(const std::string&n=""):Base(Classname::__classinfo__(),n){} \
									Classname(Miyuki::Reflection::Class * info, const std::string&n=""):Base(info,n){}
#define MYK_FINAL_CLASS(Classname, Base) MYK_CLASS_TYPE_INFO(Classname, Base) \
									Classname(const std::string&n=""):Base(Classname::__classinfo__(),n){}
#endif