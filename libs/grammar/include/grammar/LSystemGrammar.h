#pragma once

#include <optional>
#include <random>
#include <string>

#include "core/types.h"
#include "grammar/Rule.h"

namespace D {

// How context symbols in rules are matched against the derivation string.
enum class ContextMode {
  // Structural mode: push/pop symbols define branch topology; symbols in
  // `ignore` are skipped linearly. push is transparent going left; pop stops
  // right-context search at the branch boundary (unless includeSiblings=true).
  Biological,

  // Match the literal left/right neighbour — ignore, push, pop are not applied.
  Strict,
};

struct LSystemGrammar final {
  Word axiom;
  std::vector<Rule> rules;

  // Biological mode context settings.
  // Ref: https://www.sidefx.com/docs/houdini/nodes/sop/lsystem.html
  //   "Context Ignore"            -> ignore
  //   "Context Includes Siblings" -> includeSiblings
  std::string ignore;            // symbols skipped during context search (e.g. "+-|")
  std::optional<char> push;      // branch-open symbol  (e.g. '[')
  std::optional<char> pop;       // branch-close symbol (e.g. ']')
  bool includeSiblings = false;  // if true, right-context search crosses branch boundaries

  ContextMode contextMode = ContextMode::Biological;

  [[nodiscard]] bool valid() const;

  [[nodiscard]] Word derive(const Word& current) const;
  [[nodiscard]] Word derive(const Word& current, std::mt19937& rng) const;
};

}  // namespace D
