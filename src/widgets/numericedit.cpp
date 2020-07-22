#include "widgets/numericedit.h"

namespace omm
{

AbstractNumericEdit::AbstractNumericEdit(QWidget* parent) : QLineEdit(parent)
{
  setReadOnly(true);
  setFocusPolicy(Qt::StrongFocus);
}

void AbstractNumericEdit::set_prefix(const QString& prefix)
{
  m_prefix = prefix;
  update_text();
}

void AbstractNumericEdit::set_suffix(const QString& suffix)
{
  m_suffix = suffix;
  update_text();
}

void AbstractNumericEdit::paintEvent(QPaintEvent* e)
{
  QLineEdit::paintEvent(e);
}

void AbstractNumericEdit::focusOutEvent(QFocusEvent* e)
{
  setReadOnly(true);
  QLineEdit::focusOutEvent(e);
}

void AbstractNumericEdit::focusInEvent(QFocusEvent* e)
{
  const auto areasons = { Qt::TabFocusReason, Qt::BacktabFocusReason };
  if (::contains(areasons, e->reason())) {
    setReadOnly(false);
  }
  QLineEdit::focusInEvent(e);
}

void AbstractNumericEdit::mouseDoubleClickEvent(QMouseEvent*)
{
  setReadOnly(false);
  selectAll();
}

}  // namespace omm

