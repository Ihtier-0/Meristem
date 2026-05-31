#pragma once

#include <cassert>
#include <string_view>

#include "core/types.h"

namespace D {

using Condition = std::function<bool(std::span<const ParamValue>)>;
using Production = std::function<Word(std::span<const ParamValue>)>;

// [leftContext <] predecessor [> rightContext] -> successor
//
// leftContext / rightContext may hold more than one symbol, e.g. "FF" matches
// two stem segments in a row. They are read in string order: the char nearest
// the predecessor is leftContext.back() / rightContext.front(). An empty string
// means "no context on that side".
struct Rule final {
  char predecessor;
  std::string leftContext;
  std::string rightContext;
  Condition condition;
  float probability = 1.f;
  Production successor;
};

// ── Builder ───────────────────────────────────────────────────────────────────
//   ruleFor('A').to("AB")
//   ruleFor('A').to("AB").withProbability(0.6f)
//   ruleFor('A').withLeftContext('F').to("B")
//   ruleFor('A').withLeftContext("FF").to("B")
//   ruleFor('A').withLeftContext('[').withRightContext(']').to("B")

class RuleBuilder final {
 public:
  RuleBuilder& to(std::string_view succ) {
    Word word = w(succ);
    m_rule.successor = [word = std::move(word)](std::span<const ParamValue>) -> Word {
      return word;
    };
    return *this;
  }
  RuleBuilder& withProbability(float p) {
    m_rule.probability = p;
    return *this;
  }
  RuleBuilder& withLeftContext(std::string_view c) {
    m_rule.leftContext = std::string(c);
    return *this;
  }
  RuleBuilder& withLeftContext(char c) {
    m_rule.leftContext = std::string(1, c);
    return *this;
  }
  RuleBuilder& withRightContext(std::string_view c) {
    m_rule.rightContext = std::string(c);
    return *this;
  }
  RuleBuilder& withRightContext(char c) {
    m_rule.rightContext = std::string(1, c);
    return *this;
  }

  [[nodiscard]] operator Rule() const {
    assert(m_rule.predecessor != '\0' && "ruleFor() called with null predecessor");
    assert(m_rule.successor && "missing .to(...) call on RuleBuilder");
    return m_rule;
  }

 private:
  RuleBuilder() = default;

  Rule m_rule;

  friend RuleBuilder ruleFor(char pred);
};

[[nodiscard]] inline RuleBuilder ruleFor(char pred) {
  assert(pred != '\0' && "predecessor must not be null");
  RuleBuilder rb;
  rb.m_rule.predecessor = pred;
  return rb;
}

}  // namespace D
