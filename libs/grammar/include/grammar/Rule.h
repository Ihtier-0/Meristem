#pragma once

#include "core/types.h"

namespace D {

using Condition = std::function<bool(std::span<const ParamValue>)>;
using Production = std::function<Word(std::span<const ParamValue>)>;

struct Rule {
  char predecessor;
  std::optional<char> leftContext;
  std::optional<char> rightContext;
  Condition condition;      // nullptr = always fires
  float probability = 1.f;
  Production successor;
};

}  // namespace D
