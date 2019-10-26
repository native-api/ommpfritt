#include "tools/tool.h"

#include <memory>
#include "renderers/painter.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"
#include <QApplication>
#include <QKeyEvent>

namespace omm
{

Tool::Tool(Scene& scene) : PropertyOwner(&scene)
{
}

ObjectTransformation Tool::transformation() const { return ObjectTransformation(); }

bool Tool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  for (auto it = handles.rbegin(); it != handles.rend(); ++it) {
    assert(*it != nullptr);
    (*it)->mouse_move(delta, pos, e);
    switch ((*it)->status()) {
    case Handle::Status::Active:
      return true;
    case Handle::Status::Hovered:
    case Handle::Status::Inactive:
      break;
    }
  }
  return false;
}

bool Tool::mouse_press(const Vec2f& pos, const QMouseEvent& e)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  LINFO << "mouse press";
  for (auto it = handles.rbegin(); it != handles.rend(); ++it) {
    LINFO << typeid(**it).name();
    if ((*it)->mouse_press(pos, e, true)) {
      return true;
    }
  }
  return false;
}

void Tool:: mouse_release(const Vec2f& pos, const QMouseEvent& e)
{
  for (auto it = handles.rbegin(); it != handles.rend(); ++it) {
    (*it)->mouse_release(pos, e);
  }
}

void Tool::draw(Painter& renderer) const
{
  if (!!(renderer.category_filter & Painter::Category::Handles)) {
    QPainter& painter = *renderer.painter;
    painter.save();
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    for (auto&& handle : handles) {
      painter.save();
      painter.setBrush(Qt::NoBrush);
      painter.setPen(Qt::NoPen);
      handle->draw(painter);
      painter.restore();
    }
    painter.restore();
  }
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent);
  return nullptr;
}

bool Tool::integer_transformation() const
{
  return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool Tool::key_press(const QKeyEvent &event)
{
  if (event.key() == Qt::Key_Escape) {
    cancel();
    return true;
  } else {
    return false;
  }
}

void Tool::cancel()
{
  for (auto&& handle : handles) {
    handle->deactivate();
  }
}

void Tool::end() {}

std::string Tool::name() const
{
  return QCoreApplication::translate("any-context", type().c_str()).toStdString();
}

bool Tool::modifies_points() const { return false; }

}  // namespace omm
