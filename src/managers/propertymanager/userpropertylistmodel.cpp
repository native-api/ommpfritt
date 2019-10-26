#include "managers/propertymanager/userpropertylistmodel.h"
#include "aspects/propertyowner.h"
#include <variant>

namespace omm
{

UserPropertyListModel::UserPropertyListModel(AbstractPropertyOwner &owner)
{
  std::list<UserPropertyListItem> items;
  for (const std::string& key : owner.properties().keys()) {
    Property* property = owner.property(key);
    if (property->is_user_property()) {
      UserPropertyListItem item(property);
      items.push_back(item);
    }
  }
  m_items = std::vector(items.begin(), items.end());
}

int UserPropertyListModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)
  assert(!parent.isValid());
  return m_items.size();
}

QVariant UserPropertyListModel::data(const QModelIndex &index, int role) const
{
  const auto get_label = [this, index]() { return m_items.at(index.row()).label(); };

  switch (role) {
  case Qt::EditRole:
    return QString::fromStdString(get_label());;
  case Qt::DisplayRole:
    if (const std::string label = get_label(); label.empty()) {
      return tr("<unnamed property>");
    } else {
      return QString::fromStdString(label);
    }
  default:
    return QVariant();
  }
}

Qt::ItemFlags UserPropertyListModel::flags(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
      | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

UserPropertyListItem *UserPropertyListModel::item(const QModelIndex &index)
{
  if (index.row() < 0 || index.row() > static_cast<int>(m_items.size())) {
    return nullptr;
  } else {
    assert(index.column() == 0);
    assert(!index.parent().isValid());
    return &m_items.at(index.row());
  }
}

bool UserPropertyListModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
  assert(index.column() == 0);
  assert(!index.parent().isValid());
  if (role == Qt::EditRole) {
    m_items.at(index.row()).configuration["label"] = data.toString().toStdString();
    Q_EMIT dataChanged(index, index, { Qt::DisplayRole });
    return true;
  }

  return false;
}

std::vector<const UserPropertyListItem *> UserPropertyListModel::items() const
{
  return ::transform<const UserPropertyListItem*>(m_items, [](const UserPropertyListItem& item) {
    return &item;
  });
}

bool UserPropertyListModel::contains(const Property *p) const
{
  return std::find_if(m_items.begin(), m_items.end(), [p](const UserPropertyListItem& i) {
    return i.property() == p;
  }) != m_items.end();
}

void UserPropertyListModel::add_property(const QString& type)
{
  const int row = m_items.size();
  beginInsertRows(QModelIndex(), row, row);
  UserPropertyListItem item;
  item.configuration["type"] = type.toStdString();
  m_items.push_back(item);
  endInsertRows();
}

void UserPropertyListModel::del_property(const QModelIndex &index)
{
  const int row = index.row();
  beginRemoveRows(QModelIndex(), row, row);
  m_items.erase(m_items.begin() + row);
  endRemoveRows();
}

UserPropertyListItem::UserPropertyListItem(Property *property) : m_property(property)
{
  if (property != nullptr) {
    configuration = property->configuration;
  }
}

std::string UserPropertyListItem::label() const
{
  const auto label_it = configuration.find("label");
  const auto* label = label_it == configuration.end() ? nullptr
                                                      : std::get_if<std::string>(&label_it->second);
  if (label == nullptr || label->empty()) {
    return property() == nullptr ? "" : property()->label();
  } else {
    return *label;
  }
}

std::string UserPropertyListItem::type() const
{
  if (property() == nullptr) {
    // if property is null, then there must be a string-typed "type" item in configuration.
    return *std::get_if<std::string>(&configuration.at("type"));
  } else {
    return property()->type();
  }
}

}  // namespace omm
