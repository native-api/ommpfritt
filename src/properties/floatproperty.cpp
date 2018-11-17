#include "properties/floatproperty.h"

namespace omm
{

std::string FloatProperty::type() const
{
  return "FloatProperty";
}

std::string FloatProperty::widget_type() const
{
  return "FloatPropertyWidget";
}

void FloatProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set_value(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_double(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void FloatProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

}  // namespace omm