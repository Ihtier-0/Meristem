#pragma once

#include "core/types.h"

namespace D {

struct TreeNode {
  Vec3 position = {0.f, 0.f, 0.f};
  float radius = 0.1f;
  int parent = -1;  // -1 = root
};

struct TreeGraph {
  std::vector<TreeNode> nodes;
};

}  // namespace D
