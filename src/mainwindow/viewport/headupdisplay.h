#pragma once

#include <QPoint>
#include <QSize>

class QPainter;
class QMouseEvent;

namespace omm
{

class HeadUpDisplay
{
public:
  QPoint pos;
  virtual ~HeadUpDisplay() = default;
  virtual QSize size() const = 0;
  virtual void draw(QPainter& painter) const = 0;
  virtual bool mouse_press(QMouseEvent& event) = 0;
  virtual void mouse_release(QMouseEvent& event) = 0;
  virtual bool mouse_move(QMouseEvent& event) = 0;
};

}  // namespace omm
