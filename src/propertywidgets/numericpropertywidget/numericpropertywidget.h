#pragma once

#include "propertywidgets/propertywidget.h"
#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/numericpropertywidget/numericmultivalueedit.h"
#include "properties/integerproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

template<typename NumericPropertyT>
class NumericPropertyWidget : public PropertyWidget<NumericPropertyT>
{
public:
  using value_type = typename NumericPropertyT::value_type;
  explicit NumericPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;

private:
  NumericMultiValueEdit<value_type>* m_spinbox;
};

}  // namespace omm
