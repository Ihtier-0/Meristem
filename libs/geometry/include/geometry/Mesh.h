#pragma once

#include <cstdint>
#include <vector>

#include "core/math.h"

namespace D {

enum class PrimitiveMode : uint8_t { Lines, Triangles };

struct Mesh {
  std::vector<Vec3> positions;
  std::vector<uint32_t> indices;
  PrimitiveMode mode = PrimitiveMode::Triangles;

  bool empty() const { return positions.empty(); }
};

}  // namespace D
