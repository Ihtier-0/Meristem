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

// Context-sensitive (1L): classic ABP fractal plant with one context rule.
// Base rule:    X → F+[[X]-X]-F[-FX]+X   (left-leaning sub-tree)
// Context rule: F < X → F-[[X]+X]+F[+FX]-X  (X after F: mirrored sub-tree)
// The context rule fires for the FX pattern inside each [-FX] node,
// making those branches right-leaning instead of left-leaning.
// Result: an organic, asymmetric plant. Press Step 3-5 times.
inline LSystemGrammar contextSensitivePlant() {
  LSystemGrammar g;
  g.angle = 25.f;
  g.axiom = {Symbol('X')};

  // F < X → F-[[X]+X]+F[+FX]-X  (context rule — MUST come before default)
  g.rules.push_back({
      .predecessor = 'X',
      .leftContext  = 'F',
      .successor   = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('-'), Symbol('['), Symbol('['),
                Symbol('X'), Symbol(']'), Symbol('+'), Symbol('X'),
                Symbol(']'), Symbol('+'), Symbol('F'), Symbol('['),
                Symbol('+'), Symbol('F'), Symbol('X'), Symbol(']'),
                Symbol('-'), Symbol('X')};
      },
  });

  // X → F+[[X]-X]-F[-FX]+X  (default)
  g.rules.push_back({
      .predecessor = 'X',
      .successor   = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('+'), Symbol('['), Symbol('['),
                Symbol('X'), Symbol(']'), Symbol('-'), Symbol('X'),
                Symbol(']'), Symbol('-'), Symbol('F'), Symbol('['),
                Symbol('-'), Symbol('F'), Symbol('X'), Symbol(']'),
                Symbol('+'), Symbol('X')};
      },
  });

  // F → FF
  g.rules.push_back({
      .predecessor = 'F',
      .successor   = [](std::span<const ParamValue>) -> Word {
        return {Symbol('F'), Symbol('F')};
      },
  });

  return g;
}

}  // namespace D::examples
