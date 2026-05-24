#pragma once

#include "geometry/Mesh.h"

namespace D {

struct DrawCall {
  const Mesh* mesh = nullptr;
  Mat4 transform{1.f};
  Vec4 color = {1.f, 1.f, 1.f, 1.f};
};

}  // namespace D
