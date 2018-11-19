#pragma once

#include <set>
#include <armadillo>

#include "geometry/objecttransformation.h"
#include "renderers/style.h"
#include "mainwindow/viewport/subhandle.h"

namespace omm
{

class Object;
class AbstractRenderer;

class Handle
{
public:
  explicit Handle(const std::set<Object*>& selection = {});

  /**
   * @brief returns true if any grabbable was moved
   */
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos);

  /**
   * @brief returns true if any grabbable was hit
   */
  bool mouse_press(const arma::vec2& pos);

  void mouse_release();
  void draw(AbstractRenderer& renderer) const;

protected:
  virtual ObjectTransformation transformation() const = 0;
  const std::set<Object*>& objects() const;
  std::vector<std::unique_ptr<SubHandle>> m_sub_handles;

private:
  std::set<Object*> m_objects;
  arma::vec2 map_to_handle_local(const arma::vec2& pos) const;
};

class GlobalOrientedHandle : public Handle
{
protected:
  using Handle::Handle;
  ObjectTransformation transformation() const override;
};

}  // namespace omm
