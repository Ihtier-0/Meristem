#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace D {

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

using ParamValue = std::variant<float, int, bool>;
using ParamList = std::vector<ParamValue>;

struct Symbol {
  char letter;
  ParamList params;

  explicit Symbol(char c) : letter(c) {}
  Symbol(char c, ParamList p) : letter(c), params(std::move(p)) {}
};

using Word = std::vector<Symbol>;

}  // namespace D
