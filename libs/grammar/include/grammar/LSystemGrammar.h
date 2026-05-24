#pragma once

#include <random>

#include "core/types.h"
#include "grammar/Rule.h"

namespace D {

class LSystemGrammar {
 public:
  Word axiom;
  std::vector<Rule> rules;
  float angle = 25.f;   // δ in degrees, used by turtle interpreter
  float stepLen = 1.f;  // base step length, used by turtle interpreter

  // Deterministic derivation: first matching rule fires.
  Word derive(const Word& current) const;

  // Stochastic derivation: among matching rules, pick by probability weights.
  Word derive(const Word& current, std::mt19937& rng) const;
};

}  // namespace D
