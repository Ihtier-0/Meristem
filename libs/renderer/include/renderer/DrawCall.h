#pragma once

#include <glm/glm.hpp>

#include "geometry/Mesh.h"

namespace D {

struct DrawCall {
  const Mesh* mesh = nullptr;
  glm::mat4 transform = glm::mat4(1.f);
  glm::vec4 color = {1.f, 1.f, 1.f, 1.f};
};

}  // namespace D
