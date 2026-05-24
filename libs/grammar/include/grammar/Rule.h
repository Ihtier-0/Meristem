#pragma once

#include <string_view>

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

// ── Builder ───────────────────────────────────────────────────────────────────
//   ruleFor('A').to("AB")
//   ruleFor('A').to("AB").withProbability(0.6f)
//   ruleFor('A').withLeftContext('F').to("B")
//   ruleFor('A').withLeftContext('[').withRightContext(']').to("B")

struct RuleBuilder {
  Rule m_rule;

  RuleBuilder& to(std::string_view succ) {
    std::string s(succ);
    m_rule.successor = [s](std::span<const ParamValue>) -> Word {
      Word w;
      w.reserve(s.size());
      for (char c : s) w.emplace_back(c);
      return w;
    };
    return *this;
  }
  RuleBuilder& withProbability(float p)  { m_rule.probability = p;  return *this; }
  RuleBuilder& withLeftContext(char c)   { m_rule.leftContext  = c;  return *this; }
  RuleBuilder& withRightContext(char c)  { m_rule.rightContext = c;  return *this; }

  operator Rule() const { return m_rule; }
};

inline RuleBuilder ruleFor(char pred) {
  RuleBuilder rb;
  rb.m_rule.predecessor = pred;
  return rb;
}

}  // namespace D
