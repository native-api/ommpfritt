#pragma once

#include <Qt>
#include <QPalette>
#include "geometry/objecttransformation.h"
#include "common.h"

class QMouseEvent;

namespace omm
{

class Painter;
class Tool;

enum class AxisHandleDirection { X, Y };

class Handle
{
public:
  explicit Handle(Tool& tool);
  virtual ~Handle() = default;
  virtual void draw(QPainter& renderer) const = 0;
  virtual bool mouse_press(const Vec2f& pos, const QMouseEvent& event);
  virtual bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&);
  virtual void mouse_release(const Vec2f& pos, const QMouseEvent&);
  HandleStatus status() const;
  virtual void deactivate();
  virtual double draw_epsilon() const;
  virtual double interact_epsilon() const;

protected:
  virtual bool contains_global(const Vec2f& global_point) const = 0;
  Tool& tool;
  Vec2f press_pos() const;

  [[nodiscard]] Vec2f discretize(const Vec2f& vec, bool local, double step) const;
  [[nodiscard]] double discretize(const double s, double step) const;

  QColor ui_color(HandleStatus status, const QString& name) const;
  QColor ui_color(const QString& name) const;

private:
  HandleStatus m_status = HandleStatus::Inactive;
  bool m_enabled = false;
  Vec2f m_press_pos;

public:
  static const std::map<AxisHandleDirection, Vec2f> axis_directions;
  static const std::map<AxisHandleDirection, QString> axis_names;
};

}  // namespace omm
