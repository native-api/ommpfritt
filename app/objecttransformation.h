#pragma once

#include <armadillo>
#include "external/json_fwd.hpp"

namespace omm {

class ObjectTransformation : public arma::Mat<double>::fixed<3, 3>
{
public:
  using arma::Mat<double>::fixed<3, 3>::fixed;

  nlohmann::json to_json() const;
};

}  // namespace omm