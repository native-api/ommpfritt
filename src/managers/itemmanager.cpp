#include "managers/itemmanager.h"

#include <QItemSelectionModel>
#include <QEvent>
#include "scene/scene.h"
#include "managers/stylemanager/stylelistview.h"
#include "managers/stylemanager/stylelistadapter.h"
#include "managers/objectmanager/objecttreeview.h"
#include "managers/objectmanager/objecttreeadapter.h"
#include "renderers/style.h"
#include "objects/object.h"

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
ItemManager<ItemViewT, ItemModelT>::ItemManager(const QString& title, Scene& scene)
  : Manager(title, scene)
  , m_item_model(scene)
{
  auto view = std::make_unique<ItemViewT>();
  view->set_model(&m_item_model);

  connect( view->selectionModel(), &QItemSelectionModel::selectionChanged,
           this, &ItemManager::on_selection_changed );

  view->installEventFilter(this);
  setWidget(view.release());
}

template<typename ItemViewT, typename ItemModelT> bool
ItemManager<ItemViewT, ItemModelT>::eventFilter(QObject* object, QEvent* event)
{
  if (object == widget() && event->type() == QEvent::MouseButtonRelease) {
    m_scene.selection_changed();
  }
  return Manager::eventFilter(object, event);
}

template<typename ItemViewT, typename ItemModelT> void
ItemManager<ItemViewT, ItemModelT>::on_selection_changed( const QItemSelection& selection,
                                                          const QItemSelection& old_selection )
{
  for (auto& index : old_selection.indexes()) {
    if (!selection.contains(index)) {
      m_item_model.item_at(index).deselect();
    }
  }
  for (auto& index : selection.indexes()) {
    m_item_model.item_at(index).select();
  }
}

template class ItemManager<ObjectTreeView, ObjectTreeAdapter>;
template class ItemManager<StyleListView, StyleListAdapter>;

}  // namespace omm
