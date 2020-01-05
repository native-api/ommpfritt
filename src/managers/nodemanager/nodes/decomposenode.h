#pragma once

#include "managers/nodemanager/node.h"

namespace omm
{

template<PortType> class OrdinaryPort;

class DecomposeNode : public Node
{
public:
  explicit DecomposeNode(Scene* scene);
  static constexpr auto INPUT_PROPERTY_KEY = "in";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "DecomposeNode");

  QString definition() const override;
  QString output_data_type(const OutputPort& port) const override;
  QString title() const override;
  bool accepts_input_data_type(const QString& type, const InputPort& port) const override;
  QString type() const override { return TYPE; }

private:
  OutputPort* m_output_x_port;
  OutputPort* m_output_y_port;
};

}  // namespace omm