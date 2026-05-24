#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "core/math.h"

namespace D {

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
