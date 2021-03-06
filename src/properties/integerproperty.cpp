#include "properties/integerproperty.h"

namespace omm
{

const Property::PropertyDetail IntegerProperty::detail
{
  [](const Property&, std::size_t) { return ""; }
};

void IntegerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_int(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
      deserializer.get_int(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void IntegerProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value( default_value(),
                          make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  }
}

IntegerProperty& IntegerProperty::set_special_value(const QString& label)
{
  special_value_label = label;
  return *this;
}

}  // namespace omm
