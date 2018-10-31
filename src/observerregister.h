#pragma once

#include <unordered_set>

template<typename ObserverT> class ObserverRegister
{
public:
  void register_observer(ObserverT& observer)
  {
    assert(m_observers.count(&observer) == 0);
    m_observers.insert(&observer);
  }

  void unregister_observer(ObserverT& observer)
  {
    assert(m_observers.count(&observer) == 1);
    m_observers.erase(m_observers.find(&observer));
  }

  template<typename F> void for_each(F f)
  {
    std::for_each(m_observers.begin(), m_observers.end(), f);
  }

private:
  std::unordered_set<ObserverT*> m_observers;
};