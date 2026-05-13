#pragma once

#include <random>

#include "core/types.h"
#include "grammar/Rule.h"

namespace D {

class LSystemGrammar {
 public:
  Word axiom;
  std::vector<Rule> rules;
  float angle = 25.f;  // δ in degrees, used by turtle interpreter

  Word derive(const Word& current) const;
  Word deriveN(int n) const;
  Word deriveStochastic(const Word& current, std::mt19937& rng) const;
};

}  // namespace D
