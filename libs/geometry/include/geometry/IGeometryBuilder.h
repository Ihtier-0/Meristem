#pragma once

#include "geometry/Mesh.h"
#include "structure/StringStructure.h"

namespace D {

class IGeometryBuilder {
 public:
  virtual ~IGeometryBuilder() = default;

  virtual Mesh build(const StringStructure&) = 0;
};

}  // namespace D
