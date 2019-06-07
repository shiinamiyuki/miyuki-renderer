#ifndef MIYUKI_REFLECTION_REFLECTION_HPP
#define MIYUKI_REFLECTION_REFLECTION_HPP

#include "object.hpp"
#include "primitive.hpp"

#define MYK_DECL_PROPERTY(Type, Name)  Miyuki::Reflection::PropertyT<Miyuki::Reflection::_ConvertToPrimitiveType<Type>::type> Name
#define MYK_CLASS_INFO_BEGIN() const std::vector<Miyuki::Reflection::Property*> subnodes()const override{\
									std::vector<Miyuki::Reflection::Property*> __vec;	
#define MYK_ADD_PROPERTY(Name) __vec.emplace_back(&Name)
#define MYK_CLASS_INFO_END() return __vec;}
#define MYK_CLASS(Classname, Base) MYK_CLASS_TYPE_INFO(Classname, Base) \
									Classname():Base(Classname::__class__()){}
#endif