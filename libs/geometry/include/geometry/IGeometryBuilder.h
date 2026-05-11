#pragma once

#include "geometry/Mesh.h"
#include "structure/PlantStructure.h"

namespace D {

class IGeometryBuilder {
 public:
  virtual ~IGeometryBuilder() = default;

  virtual Mesh build(const StringStructure&) = 0;
  virtual Mesh build(const TreeGraph&) = 0;
};

inline Mesh buildMesh(IGeometryBuilder& builder, const PlantStructure& structure) {
  return std::visit([&](const auto& s) { return builder.build(s); }, structure);
}

}  // namespace D
