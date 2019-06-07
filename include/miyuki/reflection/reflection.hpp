#ifndef MIYUKI_REFLECTION_REFLECTION_HPP
#define MIYUKI_REFLECTION_REFLECTION_HPP

#include "object.hpp"
#include "primitive.hpp"

#define MYK_DECL_PROPERTY(Type, Name)  Miyuki::Reflection::PropertyT<Miyuki::Reflection::_ConvertToPrimitiveType<Type>::type> Name
#define MYK_PROPERTY_BEGIN() const std::vector<Miyuki::Reflection::Property*> getProperties()const override{\
									std::vector<Miyuki::Reflection::Property*> __vec;	
#define MYK_ADD_PROPERTY(Name) __vec.emplace_back(&Name)
#define MYK_PROPERTY_END() return __vec;}
#define MYK_CLASS(Classname, Base) MYK_CLASS_TYPE_INFO(Classname, Base) \
									Classname(const std::string&n=""):Base(Classname::__classinfo__(),n){} \
									Classname(Miyuki::Reflection::Class * info, const std::string&n=""):Base(info,n){}
#define MYK_FINAL_CLASS(Classname, Base) MYK_CLASS_TYPE_INFO(Classname, Base) \
									Classname(const std::string&n=""):Base(Classname::__classinfo__(),n){}
#endif