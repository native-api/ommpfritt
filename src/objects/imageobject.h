#pragma once

#include "objects/object.h"

namespace omm
{

class ImageObject : public Object
{
public:
  explicit ImageObject(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) override;
  std::string type() const override;
  BoundingBox bounding_box() override;
  std::unique_ptr<Object> clone() const override;
  static constexpr auto TYPE = "ImageObject";
  static constexpr auto FILEPATH_PROPERTY_KEY = "filename";
};

}  // namespace omm