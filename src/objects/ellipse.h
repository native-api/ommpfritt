#pragma once

#include "objects/abstractpath.h"
#include <Qt>

namespace omm
{

class Scene;

class Ellipse : public AbstractPath
{
public:
  explicit Ellipse(Scene* scene);
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Ellipse");
  Flag flags() const override;

  static constexpr auto RADIUS_PROPERTY_KEY = "r";
  static constexpr auto CORNER_COUNT_PROPERTY_KEY = "n";
  static constexpr auto SMOOTH_PROPERTY_KEY = "smooth";
  void on_property_value_changed(Property* property) override;
  bool is_closed() const override { return true; }

private:
  Geom::PathVector paths() const override;
};

}  // namespace omm
