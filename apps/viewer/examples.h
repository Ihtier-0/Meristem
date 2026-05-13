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

// Stochastic plant: F has two competing productions (p=0.6 / p=0.4), delta = 25 deg
//   F -p0.6-> F[+F]F[-F]F
//   F -p0.4-> F[+F]F
inline LSystemGrammar stochasticPlant() {
  LSystemGrammar g;
  g.angle = 25.f;
  g.axiom = {Symbol('F')};

  g.rules.push_back({
      .predecessor  = 'F',
      .probability  = 0.6f,
      .successor    = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('['), Symbol('+'), Symbol('F'),
                Symbol(']'), Symbol('F'), Symbol('['), Symbol('-'),
                Symbol('F'), Symbol(']'), Symbol('F')};
      },
  });

  g.rules.push_back({
      .predecessor  = 'F',
      .probability  = 0.4f,
      .successor    = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('['), Symbol('+'), Symbol('F'),
                Symbol(']'), Symbol('F')};
      },
  });

  return g;
}

}  // namespace D::examples
