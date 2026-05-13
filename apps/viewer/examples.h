#pragma once

#include "grammar/LSystemGrammar.h"

namespace D::examples {

// A -> F[+A][-A], F -> FF, delta = 25 deg
inline LSystemGrammar binaryTree() {
  LSystemGrammar g;
  g.angle = 25.f;
  g.axiom = {Symbol('A')};

  g.rules.push_back({
      .predecessor = 'A',
      .successor = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('['), Symbol('+'), Symbol('A'),
                Symbol(']'), Symbol('['), Symbol('-'), Symbol('A'), Symbol(']')};
      },
  });

  g.rules.push_back({
      .predecessor = 'F',
      .successor = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('F')};
      },
  });

  return g;
}

}  // namespace D::examples
