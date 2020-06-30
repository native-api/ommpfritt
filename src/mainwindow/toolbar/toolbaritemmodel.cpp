#include "mainwindow/toolbar/toolbaritemmodel.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include "mainwindow/toolbar/toolbar.h"
#include <QMimeData>
#include <QTreeWidgetItem>
#include <QWidgetAction>
#include "mainwindow/application.h"
#include <QToolButton>

namespace
{

constexpr auto type_key = "type";
constexpr auto action_item_id = QStandardItem::UserType + 1;
constexpr auto group_item_id = QStandardItem::UserType + 2;
constexpr auto separator_item_id = QStandardItem::UserType + 3;

using namespace omm;

class AbstractItem : public QStandardItem
{
protected:
  using QStandardItem::QStandardItem;
  explicit AbstractItem() : QStandardItem(0, 1)
  {
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  }
public:
  virtual nlohmann::json encode() const = 0;
  virtual std::unique_ptr<QAction> make_action() const = 0;
};

template<int item_id>
class Item : public AbstractItem
{
public:
  static constexpr auto TYPE = item_id;
  int type() const override { return TYPE; }
protected:
  using AbstractItem::AbstractItem;
};

class ActionItem : public Item<action_item_id>
{
public:
  explicit ActionItem(const QString& command_name) : command_name(command_name)
  {
    const auto& key_bindings = omm::Application::instance().key_bindings;
    const auto* item = key_bindings.value(omm::Application::TYPE, command_name);
    setData(item->translated_name(omm::KeyBindings::TRANSLATION_CONTEXT), Qt::DisplayRole);
    setData(item->icon(), Qt::DecorationRole);
  }

  std::unique_ptr<QAction> make_action() const override
  {
    auto& app = Application::instance();
    const auto& key_bindings = omm::Application::instance().key_bindings;
    return key_bindings.make_toolbar_action(app, command_name);
  }

  nlohmann::json encode() const override
  {
    return command_name.toStdString();
  }

private:
  const QString command_name;
};

template<int type>
class HyperItem : public Item<type>
{
public:
  HyperItem(const QString& label)
  {
    this->setData(label, Qt::DisplayRole);
    this->setFlags(this->flags() | Qt::ItemIsDropEnabled);
  }

protected:
  nlohmann::json encode() const override { return {{type_key, type}}; }
  std::unique_ptr<QToolButton> children_toolbutton() const
  {
    auto button = std::make_unique<QToolButton>();
    QObject::connect(button.get(), &QToolButton::triggered,
                     button.get(), &QToolButton::setDefaultAction);
    for (int row = 0; row < this->rowCount(); ++row) {
      const auto item = this->child(row);
      button->addAction(static_cast<const AbstractItem*>(item)->make_action().release());
    }
    button->setDefaultAction(button->actions().first());
    return button;
  }
};

class GroupItem : public HyperItem<group_item_id>
{
public:
  explicit GroupItem() : HyperItem(omm::ToolBarItemModel::tr("group")) {}

  nlohmann::json encode() const override
  {
    nlohmann::json items;
    for (int i = 0; i < rowCount(); ++i) {
      items.push_back(static_cast<const Item*>(child(i, 0))->encode());
    }
    auto j = HyperItem::encode();
    j[omm::ToolBarItemModel::items_key] = items;
    return j;
  }

  std::unique_ptr<QAction> make_action() const override
  {
    auto action = std::make_unique<QWidgetAction>(nullptr);
    action->setDefaultWidget(children_toolbutton().release());
    return action;
  }
};

class SeparatorItem : public HyperItem<separator_item_id>
{
public:
  explicit SeparatorItem() : HyperItem(omm::ToolBarItemModel::tr("separator")) {}

  std::unique_ptr<QAction> make_action() const override
  {
    auto action = std::make_unique<QAction>();
    action->setSeparator(true);
    return action;
  }
};

}  // namespace

namespace omm
{

nlohmann::json ToolBarItemModel::encode(const QModelIndexList& indices) const
{
  const auto filter = [indices](const QModelIndex& index) {
    const auto is_descendant_of = [index](QModelIndex potential_descendant) {
      while (true) {
        if (potential_descendant == index) {
          return true;
        } else if (potential_descendant.isValid()) {
          potential_descendant = potential_descendant.parent();
        } else {
          return false;
        }
      }
      Q_UNREACHABLE();
      return false;
    };
    return index.column() == 0 && std::none_of(indices.begin(), indices.end(), is_descendant_of);
  };

  nlohmann::json items;
  for (const QModelIndex& index : indices) {
    if (filter(index)) {
      continue;
    }
    items.push_back(static_cast<const AbstractItem*>(itemFromIndex(index))->encode());
  }
  return {
    {items_key, items}
  };
}

QString ToolBarItemModel::encode_str(const QModelIndexList& indices) const
{
  return QString::fromStdString(encode(indices).dump());
}

QString ToolBarItemModel::encode_str() const
{
  return QString::fromStdString(encode().dump());
}

nlohmann::json ToolBarItemModel::encode() const
{
  if (const std::size_t n = rowCount(); n == 0) {
    return {};
  } else {
    return encode(QItemSelectionRange(index(0, 0), index(n-1, 0)).indexes());
  }
}

void ToolBarItemModel::remove_selection(const QItemSelection& selection)
{
  auto sorted_selection = selection;
  std::sort(sorted_selection.begin(), sorted_selection.end(),
            [](const QItemSelectionRange& a, const QItemSelectionRange& b)
  {
    if (a.top() < b.top()) {
      assert(a.bottom() < b.top());  // no overlap
      return false;
    } else {
      assert(a.top() > b.bottom());  // no overlap
      return true;
    }
  });
  for (const auto& range : sorted_selection) {
    beginRemoveRows(range.parent(), range.top(), range.bottom());
    removeRows(range.top(), range.height(), range.parent());
    endRemoveRows();
  }
}

void ToolBarItemModel::add_group()
{
  add_items(nlohmann::json{ {items_key, { {
             {type_key, SeparatorItem::TYPE},
             {items_key, nlohmann::json::array()}
            } }} }, rowCount());
}

void ToolBarItemModel::add_separator()
{
  add_items(nlohmann::json{ {items_key, { {{type_key, SeparatorItem::TYPE}} }} }, rowCount());
}

void ToolBarItemModel::add_items(const QString& code, int row, const QModelIndex& parent)
{
  try {
    const auto json = nlohmann::json::parse(code.toStdString());
    add_items(json, row, parent);
  } catch (const nlohmann::json::exception& e) {
    LWARNING << "Decode ToolBar: Failed to parse json document.";
    LINFO << e.what() << " " << code;
  }
}

void ToolBarItemModel::add_items(const nlohmann::json& code, int row, const QModelIndex& parent)
{
  LINFO << code.dump();
  QList<QStandardItem*> items;
  std::list<std::pair<GroupItem*, nlohmann::json>> groups;
  for (auto&& item : code.at(items_key)) {
    switch (item.type()) {
    case nlohmann::json::value_t::object:
      switch (static_cast<int>(item[type_key])) {
      case GroupItem::TYPE:
        if (parent.isValid()) {
          LWARNING << "Nested groups are not allowed in tool bars.";
          return;
        } else {
          auto group_item = std::make_unique<GroupItem>();
          groups.emplace_back(group_item.get(), item);
          items.push_back(group_item.release());
        }
        break;
      case SeparatorItem::TYPE:
        items.push_back(std::make_unique<SeparatorItem>().release());
        break;
      default:
        Q_UNREACHABLE();
      }
      break;
    case nlohmann::json::value_t::string:
      items.push_back(std::make_unique<ActionItem>(QString::fromStdString(item)).release());
      break;
    default:
      LWARNING << "ignoring data of unexpected type: " << QString::fromStdString(item.dump());
    }
  }

  row = row < 0 ? rowCount(parent) : row;
  beginInsertRows(parent, row, row + items.size() - 1);
  if (parent.isValid()) {
    itemFromIndex(parent)->insertRows(row, items);
  } else {
    for (int i = 0; i < items.size(); ++i) {
      insertRow(row + i, items[i]);
    }
  }
  endInsertRows();

  for (const auto& [group_item, configuration] : groups) {
    const QModelIndex index = indexFromItem(group_item);
    assert(index.isValid());
    add_items(configuration, 0, index);
  }
}

void ToolBarItemModel::populate(QToolBar& tool_bar) const
{
  for (int row = 0; row < rowCount(); ++row) {
    tool_bar.addAction(static_cast<const AbstractItem*>(item(row, 0))->make_action().release());
  }
  Q_UNUSED(tool_bar)
}

void ToolBarItemModel::reset(const QString& configuration)
{
  beginResetModel();
  clear();
  add_items(configuration);
  endResetModel();
}

bool ToolBarItemModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                       int row, int column, const QModelIndex& parent) const
{
  Q_UNUSED(column)
  Q_UNUSED(row)
  Q_UNUSED(parent)
  return data->hasFormat(ToolBarDialog::mime_type)
      && ::contains(std::set{ Qt::MoveAction, Qt::LinkAction}, action);
}

bool ToolBarItemModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                    int row, int column, const QModelIndex& parent)
{
  Q_UNUSED(action)
  Q_UNUSED(column)
  assert(canDropMimeData(data, action, row, column, parent));
  QDataStream stream(data->data(ToolBarDialog::mime_type));
  QString code;
  stream >> code;
  const auto json_code = nlohmann::json::parse(code.toStdString());
  add_items(json_code, row, parent);
  return false;
}

QMimeData* ToolBarItemModel::mimeData(const QModelIndexList& indices) const
{
  QByteArray buffer;
  {
    const auto json = encode(indices);
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    const auto code = QString::fromStdString(json.dump());
    stream << code;
  }
  auto mime_data = std::make_unique<QMimeData>();
  mime_data->setData(omm::ToolBarDialog::mime_type, buffer);
  return mime_data.release();
}



}  // namespace omm
