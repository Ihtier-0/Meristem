#pragma once

#include "algorithm/ParametricLSystemAlgorithm.h"
#include "grammar/LSystemGrammar.h"

namespace D::examples {

// A -> F[+A][-A], F -> FF, delta = 25 deg
inline LSystemGrammar binaryTree() {
  LSystemGrammar g;

  g.axiom = w("A");
  g.rules = {
      ruleFor('A').to("F[+A][-A]"),
      ruleFor('F').to("FF"),
  };
  return g;
}

// Stochastic plant: F has two competing productions (p=0.6 / p=0.4), delta = 25 deg
//   F -p0.6-> F[+F]F[-F]F
//   F -p0.4-> F[+F]F
inline LSystemGrammar stochasticPlant() {
  LSystemGrammar g;

  g.axiom = w("F");
  g.rules = {
      ruleFor('F').to("F[+F]F[-F]F").withProbability(0.6f),
      ruleFor('F').to("F[+F]F").withProbability(0.4f),
  };
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

  g.axiom = w("X");
  g.ignore = "+-|";
  g.push = '[';
  g.pop = ']';
  g.rules = {
      // F < X → F-[[X]+X]+F[+FX]-X  (context rule — MUST come before default)
      ruleFor('X').withLeftContext('F').to("F-[[X]+X]+F[+FX]-X"),
      // X → F+[[X]-X]-F[-FX]+X  (default)
      ruleFor('X').to("F+[[X]-X]-F[-FX]+X"),
      // F → FF
      ruleFor('F').to("FF"),
  };
  return g;
}

// Context-sensitive (2L): ABP fractal plant base with two-sided context rules.
// In the string F+[[X]-X]-F[-FX]+X the X's have three distinct neighbour pairs:
//   [ < X > ]  — innermost nested X  → mirrored sub-tree (swap +/-)
//   F < X > ]  — X inside [-FX]      → simpler alternating pattern
//   default    — all other X's        → standard ABP formula
// Both left AND right context are checked simultaneously (true 2L).
// Press Step 3–4 times.
inline LSystemGrammar contextSensitive2LPlant() {
  LSystemGrammar g;

  g.axiom = w("X");
  // '[' and ']' are used as context chars — requires Strict mode so they are
  // matched literally rather than treated as transparent branch delimiters.
  g.contextMode = ContextMode::Strict;
  g.rules = {
      // [ < X > ] → F-[[X]+X]+F[+FX]-X  (mirrored — MUST be before default)
      ruleFor('X').withLeftContext('[').withRightContext(']').to("F-[[X]+X]+F[+FX]-X"),
      // F < X > ] → F[+X]F[-X]X  (simpler alternating — MUST be before default)
      ruleFor('X').withLeftContext('F').withRightContext(']').to("F[+X]F[-X]X"),
      // X → F+[[X]-X]-F[-FX]+X  (default ABP fractal plant)
      ruleFor('X').to("F+[[X]-X]-F[-FX]+X"),
      // F → FF
      ruleFor('F').to("FF"),
  };
  return g;
}

// Context-sensitive flower: true bottom-up signal propagation.
//
// Alphabet:
//   A  — active growing apex (main axis)
//   a  — dormant lateral bud (waits until parent stem matures)
//   i  — immature stem segment (matures to F in one step)
//   F  — mature stem
//   K  — flower (terminal)
//
// Rules (priority order matters):
//   F < a → K     context: dormant bud sees mature parent → flower
//   i < a → a     context: dormant bud sees immature parent → stays dormant
//   a   → a       default: dormant stays dormant
//   A   → i[+a][-a]A   active apex grows: immature stem + 2 dormant lateral buds + continues
//   i   → F       immature matures
//   F   → FF      mature elongates
//
// Step 0: A
// Step 1: i[+a][-a]A          — immature stem, buds dormant
// Step 2: F[+a][-a]i[+a][-a]A — base mature; bottom buds still next to old i (now F), wait...
//   actually bottom buds NOW see F → K on step 3
// Step 3: first flowers at bottom level; next level buds still dormant
// Step 4: next level flowers — propagation goes bottom to top
inline LSystemGrammar contextSensitiveFlower() {
  LSystemGrammar g;

  g.axiom = w("A");
  g.ignore = "+-|";
  g.push = '[';
  g.pop = ']';
  g.rules = {
      ruleFor('a').withLeftContext('F').to("K"),  // F < a → K  (MUST be before default 'a')
      ruleFor('a').withLeftContext('i').to("a"),  // i < a → a  (MUST be before default 'a')
      ruleFor('a').to("a"),                       // a → a  (default: stay dormant)
      ruleFor('A').to("i[+a][-a]A"),              // active apex grows
      ruleFor('i').to("F"),                       // immature matures
      ruleFor('F').to("FF"),                      // mature elongates
  };
  return g;
}

// Parametric: A(s) → F(s)[+A(s*r)][-A(s*r)]
// Each recursive branch is r-times shorter — natural tapering tree.
// Global param r=0.7 (adjustable via slider). Press Step 6-7 times.
inline ParametricLSystemAlgorithm parametricTree() {
  using PRule = ParametricLSystemAlgorithm::PRule;

  Word axiom;
  axiom.emplace_back('A', ParamList{ParamValue{1.0f}});

  std::vector<PRule> rules;
  rules.push_back({'A', {"s"}, "F(s)[+A(s*r)][-A(s*r)]"});

  ParametricLSystemAlgorithm algo(std::move(axiom), std::move(rules), 25.f);
  algo.setGlobalParams({{"r", 0.7f}});
  return algo;
}

}  // namespace D::examples
