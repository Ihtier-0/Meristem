#pragma once

#include "core/types.h"
#include "geometry/Mesh.h"

namespace D {

class IGeometryBuilder {
 public:
  virtual ~IGeometryBuilder() = default;

  virtual Mesh build(const Word&) = 0;
};

}  // namespace D
