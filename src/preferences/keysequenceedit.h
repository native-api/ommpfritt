#pragma once

#include <QKeySequence>
#include <QWidget>
#include <memory>
#include "ui_keysequenceedit.h"

namespace omm
{

class KeySequenceEdit : public QWidget
{
  Q_OBJECT
public:
  explicit KeySequenceEdit(QWidget* parent = nullptr);
  void set_key_sequence(const QKeySequence& key_sequence);
  void set_default_key_sequence(const QKeySequence& key_sequence);
  QKeySequence key_sequence() const;
  void focusInEvent(QFocusEvent *event) override;
  bool focusNextPrevChild(bool next) override;

private:
  std::unique_ptr<Ui::KeySequenceEdit> m_ui;
  QKeySequence m_default_key_sequence;
};

}  // namespace omm
