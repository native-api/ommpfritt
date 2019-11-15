#include "tools/handles/handle.h"
#include "logging.h"
#include "preferences/uicolors.h"
#include "renderers/painter.h"
#include <QMouseEvent>
#include "tools/tool.h"
#include "scene/scene.h"
#include "scene/messagebox.h"

namespace omm
{

Handle::Handle(Tool& tool) : tool(tool) {}

bool Handle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  m_press_pos = pos;
  if (contains_global(pos)) {
    if (event.button() == Qt::LeftButton) {
      m_status = HandleStatus::Active;
    }
    return true;
  } else {
    return false;
  }
}

bool Handle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent&)
{
  const auto old_status = m_status;
  Q_UNUSED(delta);
  if (m_status != HandleStatus::Active) {
    if (contains_global(pos)) {
      m_status = HandleStatus::Hovered;
    } else {
      m_status = HandleStatus::Inactive;
    }
  }
  if (m_status != old_status) {
    Q_EMIT tool.scene()->message_box().appearance_changed(tool);
  }
  return false;
}

void Handle::mouse_release(const Vec2f& pos, const QMouseEvent&)
{
  Q_UNUSED(pos);
  m_status = HandleStatus::Inactive;
}

HandleStatus Handle::status() const { return m_status; }
void Handle::deactivate() { m_status = HandleStatus::Inactive; }

double Handle::draw_epsilon() const { return 4.0; }
double Handle::interact_epsilon() const { return 4.0; }
Vec2f Handle::press_pos() const { return m_press_pos; }

void Handle::discretize(Vec2f& vec) const
{
  if (tool.integer_transformation()) {
    vec = tool.viewport_transformation.inverted().apply_to_direction(vec);
    static constexpr double step = 10.0;
    for (auto i : { 0u, 1u }) {
      vec[i] = step * static_cast<int>(vec[i] / step);
    }
    vec = tool.viewport_transformation.apply_to_direction(vec);
  }
}

QColor Handle::ui_color(HandleStatus status, const QString& name) const
{
  return omm::ui_color(status, "Handle", name);
}

QColor Handle::ui_color(const QString& name) const
{
  return ui_color(status(), name);
}

}  // namespace omm
