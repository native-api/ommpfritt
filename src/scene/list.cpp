#include "scene/list.h"
#include <algorithm>
#include "scene/contextes.h"
#include "renderers/style.h"

namespace omm
{

template<typename T> std::set<T*> List<T>::items() const
{
  return ::transform<T*, std::set>(m_items, [](const auto& item) { return item.get(); });
}

template<typename T> T& List<T>::item(size_t i) const
{
  return *m_items[i].get();
}

template<typename T> T& List<T>::insert(std::unique_ptr<T> item)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this](auto* observer){ return observer->acquire_inserter_guard(m_items.size()); }
  );
  auto& ref = *item;
  m_items.push_back(std::move(item));
  this->invalidate_recursive();
  return ref;
}

template<typename T> void List<T>::insert(std::unique_ptr<T> item, const T* predecessor)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this](auto* observer){ return observer->acquire_inserter_guard(m_items.size()); }
  );
  const auto it = m_items.begin() + (predecessor == nullptr ? 0 : position(*predecessor) + 1);
  m_items.insert(it, std::move(item));
  this->invalidate_recursive();
}

template<typename T> void List<T>::insert(ListOwningContext<T>& context)
{
  const size_t position = context.predecessor == nullptr ? 0
                                                         : this->position(*context.predecessor)+1;
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this, position](auto* observer) {
      return observer->acquire_inserter_guard(position);
    }
  );
  m_items.insert(m_items.begin() + position, context.subject.release());
  this->invalidate_recursive();
}

template<typename T> void List<T>::remove(ListOwningContext<T>& context)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this, &context](auto* observer) {
      return observer->acquire_remover_guard(position(context.subject));
    }
  );
  context.subject.capture(::extract(m_items, context.subject.get()));
  this->invalidate_recursive();
}

template<typename T> std::unique_ptr<T> List<T>::remove(T& item)
{
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this, &item](auto* observer){ return observer->acquire_remover_guard(position(item)); }
  );
  auto extracted_item = ::extract(m_items, item);
  this->invalidate_recursive();
  return  extracted_item;
}

template<typename T> size_t List<T>::position(const T& item) const
{
  const auto it = std::find_if(m_items.begin(), m_items.end(), [&item](const auto& uptr) {
    return uptr.get() == &item;
  });

  assert(it != m_items.end());
  return std::distance(m_items.begin(), it);
}

template<typename T> const T* List<T>::predecessor(const T& item) const
{
  const auto pos = this->position(item);
  if (pos == 0) {
    return nullptr;
  } else {
    return m_items.at(pos - 1).get();
  }
}

template<typename T> void List<T>::move(ListMoveContext<T>& context)
{
  assert(context.is_valid());
  const auto guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [&context](auto* observer) { return observer->acquire_mover_guard(context); }
  );

  std::unique_ptr<T> item = ::extract(m_items, context.subject.get());
  const auto pos = context.predecessor == nullptr ? 0 : position(*context.predecessor) + 1;
  m_items.insert(m_items.begin() + pos, std::move(item));
  this->invalidate_recursive();
}

template<typename T>
std::vector<std::unique_ptr<T>> List<T>::set(std::vector<std::unique_ptr<T>> items)
{
  const auto style_guards = observed_type::template transform<std::unique_ptr<AbstractRAIIGuard>>(
    [this](auto* observer) { return observer->acquire_reseter_guard(); }
  );
  auto old_items = std::move(m_items);
  m_items = std::move(items);
  this->invalidate_recursive();
  return old_items;
}

template<typename T> void List<T>::invalidate() { }

template class List<Style>;

}  // namespace omm