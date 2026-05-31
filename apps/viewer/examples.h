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

// Stochastic extension of binaryTree(), delta = 25 deg.
// Rule for A replaced by two competing productions:
//   A -p0.6-> F[+A][-A]   standard symmetric branching
//   A -p0.4-> F[+A]        only left branch (asymmetric)
// Rule F -> FF is kept. Different RNG seeds produce different bush shapes,
// demonstrating stochastic L-systems as a source of morphological variation.
inline LSystemGrammar stochasticPlant() {
  LSystemGrammar g;

  g.axiom = w("A");
  g.rules = {
      ruleFor('A').to("F[+A][-A]").withProbability(0.6f),
      ruleFor('A').to("F[+A]").withProbability(0.4f),
      ruleFor('F').to("FF"),
  };
  return g;
}

// Context-sensitive extension of binaryTree(), delta = 25 deg.
// Adds symbol K (flower). One context rule:
//   F < A -> K   apex A becomes a flower when a mature stem F is to its left
//   A -> F[+A][-A]   default branching (no context match)
//   F -> FF
//
// Derivation:
//   Step 0: A
//   Step 1: F[+A][-A]          A has no F to its left -> default fires
//   Step 2: FF[+K][-K]         both A's have F as left context -> K
//   Step 3: FFFF[+K][-K]       K is terminal (no rule); stems keep growing
//
// From the third iteration onward branch tips carry K, modelling
// bottom-up flowering signal propagation (article fig. 4).
inline LSystemGrammar contextFlower() {
  LSystemGrammar g;

  g.axiom = w("A");
  g.ignore = "+-";
  g.push = '[';
  g.pop = ']';
  g.rules = {
      ruleFor('A').withLeftContext('F').to("K"),  // F < A -> K  (before default)
      ruleFor('A').to("F[+A][-A]"),               // default branching
      ruleFor('F').to("FF"),
  };
  return g;
}

// Context-sensitive (1L): classic ABP fractal plant with one context rule.
// Base rule:    X -> F+[[X]-X]-F[-FX]+X   (left-leaning sub-tree)
// Context rule: F < X -> F-[[X]+X]+F[+FX]-X  (X after F: mirrored sub-tree)
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
      // F < X -> F-[[X]+X]+F[+FX]-X  (context rule — MUST come before default)
      ruleFor('X').withLeftContext('F').to("F-[[X]+X]+F[+FX]-X"),
      // X -> F+[[X]-X]-F[-FX]+X  (default)
      ruleFor('X').to("F+[[X]-X]-F[-FX]+X"),
      // F -> FF
      ruleFor('F').to("FF"),
  };
  return g;
}

// Context-sensitive (2L): ABP fractal plant base with two-sided context rules.
// In the string F+[[X]-X]-F[-FX]+X the X's have three distinct neighbour pairs:
//   [ < X > ]  — innermost nested X  -> mirrored sub-tree (swap +/-)
//   F < X > ]  — X inside [-FX]      -> simpler alternating pattern
//   default    — all other X's        -> standard ABP formula
// Both left AND right context are checked simultaneously (true 2L).
// Press Step 3–4 times.
inline LSystemGrammar contextSensitive2LPlant() {
  LSystemGrammar g;

  g.axiom = w("X");
  // '[' and ']' are used as context chars — requires Strict mode so they are
  // matched literally rather than treated as transparent branch delimiters.
  g.contextMode = ContextMode::Strict;
  g.rules = {
      // [ < X > ] -> F-[[X]+X]+F[+FX]-X  (mirrored — MUST be before default)
      ruleFor('X').withLeftContext('[').withRightContext(']').to("F-[[X]+X]+F[+FX]-X"),
      // F < X > ] -> F[+X]F[-X]X  (simpler alternating — MUST be before default)
      ruleFor('X').withLeftContext('F').withRightContext(']').to("F[+X]F[-X]X"),
      // X -> F+[[X]-X]-F[-FX]+X  (default ABP fractal plant)
      ruleFor('X').to("F+[[X]-X]-F[-FX]+X"),
      // F -> FF
      ruleFor('F').to("FF"),
  };
  return g;
}

// Parametric: A(s) -> F(s)[+A(s*r)][-A(s*r)]
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
