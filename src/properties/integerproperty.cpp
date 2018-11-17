#include "properties/integerproperty.h"

namespace omm
{

std::string IntegerProperty::type() const
{
  return "IntegerProperty";
}

std::string IntegerProperty::widget_type() const
{
  return "IntegerPropertyWidget";
}

void IntegerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set_value(deserializer.get_int(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_int(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void IntegerProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const 
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

}  // namespace omm