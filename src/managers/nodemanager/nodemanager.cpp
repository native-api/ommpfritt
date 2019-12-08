#include "managers/nodemanager/nodemanager.h"
#include "managers/nodemanager/nodemodel.h"
#include "ui_nodemanager.h"

namespace omm
{

NodeManager::NodeManager(Scene& scene)
  : Manager(tr("Nodes"), scene)
  , m_ui(std::make_unique<Ui::NodeManager>())
{
  setObjectName(TYPE);
  auto widget = std::make_unique<QWidget>();
  m_ui->setupUi(widget.get());
  set_widget(std::move(widget));
}

NodeManager::~NodeManager()
{
}

QString NodeManager::type() const { return TYPE; }

void NodeManager::set_model(NodeModel* model)
{
  m_ui->nodeview->set_model(model);
}

bool NodeManager::perform_action([[maybe_unused]] const QString& name)
{
  return false;
}



}  // namespace omm