#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "renderers/painter.h"
#include "geometry/util.h"
#include "tools/tool.h"
#include "preferences/uicolors.h"

namespace omm
{

enum class MoveAxisHandleDirection { X, Y };

template<typename ToolT, MoveAxisHandleDirection direction>
class MoveAxisHandle : public Handle
{
public:
  MoveAxisHandle(ToolT& tool)
    : Handle(tool)
    , m_direction(direction == MoveAxisHandleDirection::X ? Vec2f(100.0, 0.0)
                                                          : Vec2f(0.0, 100.0))
  {
  }

  bool contains_global(const Vec2f& point) const override
  {
    const Vec2f global_point = tool.transformation().inverted().apply_to_position(point);
    Vec2f v = project_onto_axis(global_point);

    // clamp v between o and m_direction
    v = v.clamped(Vec2f::min(Vec2f::o(), m_direction), Vec2f::max(Vec2f::o(), m_direction));

    return (global_point - v).euclidean_norm() < interact_epsilon();
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (status() == HandleStatus::Active) {
      const auto inv_tool_transformation = tool.transformation().inverted();
      auto total_delta = inv_tool_transformation.apply_to_direction(pos - press_pos());
      total_delta = project_onto_axis(total_delta);
      discretize(total_delta);
      {
        auto transformation = omm::ObjectTransformation().translated(total_delta);
        transformation = transformation.transformed(inv_tool_transformation);
        static_cast<ToolT&>(tool).transform_objects(transformation);
      }
      total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
      const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
      static_cast<ToolT&>(tool).tool_info = tool_info;
      return true;
    } else {
      return false;
    }
  }

  void draw(QPainter& painter) const override
  {
    painter.setTransform(tool.transformation().to_qtransform(), true);
    const double magnitude = m_direction.euclidean_norm();
    const double argument = m_direction.arg();

    const auto right = to_qpoint(PolarCoordinates(argument-0.1, magnitude*0.9).to_cartesian());
    const auto left = to_qpoint(PolarCoordinates(argument+0.1, magnitude*0.9).to_cartesian());

    static const std::map<MoveAxisHandleDirection, QString> name_map {
      { MoveAxisHandleDirection::X, "x-axis" },
      { MoveAxisHandleDirection::Y, "y-axis" },
    };
    const QString name = name_map.at(direction);


    painter.save();
    painter.setPen(ui_color(name));
    painter.drawLine(QPointF(0.0, 0.0), to_qpoint(m_direction));
    const QPointF polyline[] = { left, to_qpoint(m_direction), right, left };
    painter.drawPolyline(polyline, 4);
    painter.restore();
  }

private:
  Vec2f project_onto_axis(const Vec2f& v) const
  {
    const Vec2f s = m_direction;

    // project v onto the line through o and s
    return Vec2f::dot(v, s) / Vec2f::dot(s, s) * s;
  }

  const Vec2f m_direction;
};

}  // namespace omm
